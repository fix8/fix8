//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_PERSIST_HPP_
# define _FIX8_PERSIST_HPP_

#if defined HAVE_BDB
# include <db_cxx.h>
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Base (ABC) Persister class
class Persister
{
	Persister(const Persister&);
	Persister& operator=(const Persister&);

protected:
	bool _opened;

public:
	/// Ctor.
	Persister() : _opened() {}

	/// Dtor.
	virtual ~Persister() {}

	/// Maximum length of persisted FIX message.
	enum { MaxMsgLen = MAX_MSG_LENGTH };

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	virtual bool put(const unsigned seqnum, const f8String& what) = 0;

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum) = 0;

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	virtual bool get(const unsigned seqnum, f8String& to) const = 0;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const = 0;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	virtual unsigned get_last_seqnum(unsigned& to) const = 0;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const = 0;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const = 0;

	/*! Remove all records (excluding the sequence number record 0) from the persist database
	    \return true on success */
	virtual bool purge() { return true; }

	/// Stop the persister thread.
	virtual void stop() {}
};

//-------------------------------------------------------------------------------------------------
#if defined HAVE_BDB

/// BerkeleyDB backed message persister.
class BDBPersister : public Persister
{
	dthread<BDBPersister> _thread;

	DbEnv _dbEnv;
	Db *_db;
	f8String _dbDir, _dbFname;
	bool _wasCreated;

   struct KeyDataBuffer
   {
      union Ubuf
      {
         unsigned int_;
         char char_[sizeof(unsigned)];
         Ubuf() : int_() {}
         Ubuf(const unsigned val) : int_(val) {}
         Ubuf(const Ubuf& from) : int_(from.int_) {}
      }
      keyBuf_;
		unsigned dataBufLen_;
      char dataBuf_[MaxMsgLen];

      KeyDataBuffer() : keyBuf_(), dataBufLen_(), dataBuf_() {}
      KeyDataBuffer(const unsigned ival) : keyBuf_(ival), dataBufLen_(), dataBuf_() {}
      KeyDataBuffer(const unsigned ival, const f8String& src) : keyBuf_(ival), dataBuf_()
			{ src.copy(dataBuf_, dataBufLen_ = src.size() > MaxMsgLen ? MaxMsgLen : src.size()); }
      KeyDataBuffer(const unsigned snd, const unsigned trg) : keyBuf_(), dataBufLen_(2 * sizeof(unsigned))
		{
			unsigned *loc(reinterpret_cast<unsigned *>(dataBuf_));
			*loc++ = snd;
			*loc = trg;
		}
      KeyDataBuffer(const KeyDataBuffer& from) : keyBuf_(from.keyBuf_), dataBufLen_(from.dataBufLen_)
			{ memcpy(dataBuf_, from.dataBuf_, dataBufLen_); }

		bool empty() const { return dataBufLen_ == 0 && keyBuf_.int_ == 0; }
   };

   struct KeyDataPair
   {
      Dbt _key, _data;

      KeyDataPair(KeyDataBuffer& buf)
         : _key(buf.keyBuf_.char_, sizeof(unsigned)), _data(buf.dataBuf_, buf.dataBufLen_)
      {
         _key.set_flags(DB_DBT_USERMEM);
         _key.set_ulen(sizeof(unsigned));
         _data.set_flags(DB_DBT_USERMEM);
         _data.set_ulen(MaxMsgLen);
      }
   };

	static int bt_compare_fcn(Db *db, const Dbt *p1, const Dbt *p2)
	{
		// Returns: < 0 if a < b; = 0 if a = b; > 0 if a > b

		const unsigned& a((*reinterpret_cast<KeyDataBuffer *>(p1->get_data())).keyBuf_.int_);
		const unsigned& b((*reinterpret_cast<KeyDataBuffer *>(p2->get_data())).keyBuf_.int_);

		return a < b ? -1 : a > b ? 1 : 0;
	}

	f8_concurrent_queue<KeyDataBuffer> _persist_queue;

	bool write(const KeyDataBuffer& what)
	{
		return _persist_queue.try_push(what);
	}

public:
	/// Ctor.
	BDBPersister() : _thread(ref(*this)), _dbEnv(0), _db(new Db(&_dbEnv, 0)), _wasCreated() {}
	/// Dtor.
	virtual ~BDBPersister();

	/*! Open existing database or create new database.
	    \param dbDir database environment directory
	    \param dbFname database name
	    \param purge if true, empty database if found
	    \return true on success */
	virtual bool initialise(const f8String& dbDir, const f8String& dbFname, bool purge=false);

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method it call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/// Stop the persister thread.
	void stop() { write(KeyDataBuffer()); _thread.join(); }

	/*! Persister thread entry point.
	  \return 0 on success */
	int operator()();	// write thread
};

#endif // HAVE_BDB

//-------------------------------------------------------------------------------------------------
/// Memory based message persister.
class MemoryPersister : public Persister
{
	typedef std::map<unsigned, const f8String> Store;
	Store _store;

public:
	/// Ctor.
	MemoryPersister() {}
	/// Dtor.
	virtual ~MemoryPersister() {}

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method it call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;
};

//-------------------------------------------------------------------------------------------------
/// File persister
struct Prec
{
	Prec(const off_t offset, const int32_t size) : _offset(offset), _size(size) {}
	Prec() : _offset(), _size() {}
	off_t _offset;
	int32_t _size;

	Prec& operator=(const Prec& that)
	{
		if (this != &that)
		{
			_offset = that._offset;
			_size = that._size;
		}
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const Prec& what)
		{ return os << "offset:" << what._offset << " size:" << what._size; }
};

struct IPrec
{
	IPrec(const uint32_t seq, const off_t offset, const int32_t size)
		: _seq(seq), _prec(offset, size) {}
	IPrec() : _seq() {}
	uint32_t _seq;
	Prec _prec;

	friend std::ostream& operator<<(std::ostream& os, const IPrec& what)
		{ return os << "seq:" << what._seq << ' ' << what._prec; }
};

class FilePersister : public Persister
{
	f8String _dbFname, _dbIname;
	int _fod, _iod;
	unsigned _rotnum;
	bool _wasCreated;

	typedef std::map<uint32_t, Prec> Index;
	Index _index;

public:
	/// Ctor.
	FilePersister(unsigned rotnum=0) : _fod(-1), _iod(-1), _rotnum(rotnum), _wasCreated() {}

	/// Dtor.
	virtual ~FilePersister();

	/*! Open existing database or create new database.
	    \param dbDir database directory
	    \param dbFname database name
	    \param purge if true, empty database if found
	    \return true on success */
	virtual bool initialise(const f8String& dbDir, const f8String& dbFname, bool purge=false);

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method it call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/// Stop the persister thread.
	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_PERSIST_HPP_

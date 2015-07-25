//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

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
#ifndef FIX8_PERSIST_HPP_
#define FIX8_PERSIST_HPP_

#if defined FIX8_HAVE_BDB
# include <db_cxx.h>
#endif
#if defined HAVE_LIBMEMCACHED
# include <libmemcached/memcached.h>
#endif
#if defined FIX8_HAVE_LIBHIREDIS
# include <hiredis/hiredis.h>
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Base (ABC) Persister class
class Persister
{
protected:
	bool _opened = false;

public:
	/// Ctor.
	Persister() = default;

	/// Dtor.
	virtual ~Persister() {}

	Persister(const Persister&) = delete;
	Persister& operator=(const Persister&) = delete;

	/// Maximum length of persisted FIX message.
	enum { MaxMsgLen = FIX8_MAX_MSG_LENGTH };

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	virtual bool put(const unsigned seqnum, const f8String& what) = 0;

	/*! Persist a generic value. Depending on specialisation, provide
		  direct access to the persister implementation
	    \param key key to store
	    \param what value string
	    \return true on success */
	virtual bool put(const f8String& key, const f8String& what) { return false; }

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

	/*! Retrieve a generic persisted value. Depending on specialisation, provide
		  direct access to the persister implementation
	    \param key key to retrieve
	    \param to target value string
	    \return true on success */
	virtual bool get(const f8String& key, f8String& to) const { return false; }

	/*! Delete a generic persisted value by specified key. Depending on specialisation, provide
		  direct access to the persister implementation
	    \param key key to delete
	    \return true on success */
	virtual bool del(const f8String& key) { return false; }

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
#if defined FIX8_HAVE_BDB

/// BerkeleyDB backed message persister.
class BDBPersister : public Persister
{
	f8_thread<BDBPersister> _thread;

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

	f8_thread_cancellation_token _cancellation_token;

public:
	/// Ctor.
	BDBPersister() : _thread(std::ref(*this)), _dbEnv(0), _db(new Db(&_dbEnv, 0)), _wasCreated() {}
	/// Dtor.
	virtual ~BDBPersister();

	/*! Open existing database or create new database.
	    \param dbDir database environment directory
	    \param dbFname database name
	    \param purge if true, empty database if found
	    \return true on success */
	F8API virtual bool initialise(const f8String& dbDir, const f8String& dbFname, bool purge=false);

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	F8API virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	F8API virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	F8API virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/// Stop the persister thread.
	void stop() { write(KeyDataBuffer()); _thread.join(); }

	/*! Persister thread entry point.
	  \return 0 on success */
	F8API int operator()();	// write thread

	f8_thread_cancellation_token& cancellation_token() { return _cancellation_token;	}
};

#endif // FIX8_HAVE_BDB

//-------------------------------------------------------------------------------------------------
/// Memory based message persister.
class MemoryPersister : public Persister
{
	using Store = std::map<unsigned, const f8String>;
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
	F8API virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	F8API virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	F8API virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	F8API virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_highest_seqnum(const unsigned requested, const unsigned last) const;
};

//-------------------------------------------------------------------------------------------------
#ifndef _MSC_VER
# define O_BINARY 0
#endif

//-------------------------------------------------------------------------------------------------
#pragma pack(push, 1)
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
#pragma pack(pop)

class FilePersister : public Persister
{
	f8String _dbFname, _dbIname;
	int _fod, _iod;
	unsigned _rotnum;
	bool _wasCreated;

	using Index = std::map<uint32_t, Prec>;
	Index _index;

public:
	/// Ctor.
	FilePersister(unsigned rotnum=0) : _fod(-1), _iod(-1), _rotnum(rotnum), _wasCreated() {}

	/// Dtor.
	F8API virtual ~FilePersister();

	/*! Open existing database or create new database.
	    \param dbDir database directory
	    \param dbFname database name
	    \param purge if true, empty database if found
	    \return true on success */
	F8API virtual bool initialise(const f8String& dbDir, const f8String& dbFname, bool purge = false);

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	F8API virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	F8API virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	F8API virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	F8API virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_highest_seqnum(const unsigned requested, const unsigned last) const;
};

//-------------------------------------------------------------------------------------------------
#if defined HAVE_LIBMEMCACHED
/// memcached message persister.
class MemcachedPersister : public Persister
{
	memcached_st *_cache = nullptr;
	/// this will usually be the SessionID
	f8String _key_base;
	unsigned _server_count = 0;

public:
	/// Ctor.
	MemcachedPersister() = default;

	/// Dtor.
	F8API virtual ~MemcachedPersister();

	/*! Establish 'session' with memcached cloud
	    \param config_str memcached config string
	    \param key_base key base string for this session
	    \param purge if true, empty database if found
	    \return true on success */
	F8API virtual bool initialise(const f8String& config_str, const f8String& key_base, bool purge=false);

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	F8API virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	F8API virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	F8API virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	F8API virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;

	/*! Find the nearest highest sequence number (lower than) from the given sequence number
	    \param requested sequence number to start
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_seqnum (unsigned requested) const;

	/*! Persist a generic value.
	    \param key key to store
	    \param what value string
	    \return true on success */
	F8API virtual bool put(const f8String& key, const f8String& what);

	/*! Retrieve a generic persisted value.
	    \param key key to retrieve
	    \param to target value string
	    \return true on success */
	F8API virtual bool get(const f8String& key, f8String& to) const;

	/*! Lookup the specified value by given key
	    \param key key to find
	    \param target location for result
	    \return true if found, false if not */
	bool get_from_cache(const std::string &key, std::string &target) const
	{
		uint32_t flags(0);
		memcached_return_t rc;
		size_t value_length;

		char *value(memcached_get(_cache, key.c_str(), key.size(), &value_length, &flags, &rc));
		if (value)
		{
			target.reserve(value_length);
			target.assign(value, value + value_length);
			free(value);
			return true;
		}

		return false;
	}

	/*! Write the specified value with the given key
	    \param key key to write
	    \param source value to write
	    \return true if successful, false if not */
	bool put_to_cache(const std::string &key, const std::string &source)
	{
		return memcached_success(memcached_set(_cache, key.c_str(), key.size(), source.c_str(), source.size(), 0, 0));
	}

	/*! Generate a lookup key based on the key base and the given sequence number
	    \param seqnum sequence number portion of uniquye key
	    \return result */
	const std::string generate_seq_key(unsigned seqnum) const
	{
		std::ostringstream ostr;
		ostr << _key_base << ':' << seqnum;
		return ostr.str();
	}

	/*! Generate a control record based on the given sequence numbers
	    \param sender_seqnum sequence number
	    \param target_seqnum sequence number
	    \return result */
	static std::string generate_ctrl_record(unsigned sender_seqnum, unsigned target_seqnum)
	{
		std::ostringstream ostr;
		ostr << sender_seqnum << ':' << target_seqnum;
		return ostr.str();
	}

	/*! Extract a control record from the given string
	    \param source source string
	    \param sender_seqnum reference to target sequence number
	    \param target_seqnum reference to target sequence number
	    \return true on success */
	static bool extract_ctrl_record(const std::string& source, unsigned &sender_seqnum, unsigned &target_seqnum)
	{
		std::istringstream istr(source);
		istr >> sender_seqnum;
		istr.ignore();
		istr >> target_seqnum;
		return true;
	}
};

#endif // HAVE_LIBMEMCACHED

//-------------------------------------------------------------------------------------------------
#if defined FIX8_HAVE_LIBHIREDIS
/// redis message persister.
class HiredisPersister : public Persister
{
	redisContext *_cache = nullptr;
	/// this will usually be the SessionID
	f8String _key_base;

public:
	/// Ctor.
	HiredisPersister() = default;

	/// Dtor.
	F8API virtual ~HiredisPersister();

	/*! Establish 'session' with redis cloud
	    \param host server host to connect to
	    \param port port to connect on
	    \param connect_timeout connect timeout in secs
	    \param key_base key base string for this session
	    \param purge if true, empty database if found
	    \return true on success */
	F8API virtual bool initialise(const f8String& host, unsigned port, unsigned connect_timeout,
		const f8String& key_base, bool purge=false);

	/*! Persist a message.
	    \param seqnum sequence number of message
	    \param what message string
	    \return true on success */
	F8API virtual bool put(const unsigned seqnum, const f8String& what);

	/*! Persist a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);

	/*! Retrieve a persisted message.
	    \param seqnum sequence number of message
	    \param to target message string
	    \return true on success */
	F8API virtual bool get(const unsigned seqnum, f8String& to) const;

	/*! Retrieve a range of persisted messages.
	    \param from start at sequence number
	    \param to end sequence number
	    \param session session containing callback method
	    \param callback method to call with each retrieved message
	    \return number of messages retrieved */
	F8API virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const;

	/*! Retrieve sequence number of last peristed message.
	    \param to target sequence number
	    \return sequence number of last peristed message on success */
	F8API virtual unsigned get_last_seqnum(unsigned& to) const;

	/*! Retrieve a sequence control record.
	    \param sender_seqnum sequence number of last sent message
	    \param target_seqnum sequence number of last received message
	    \return true on success */
	F8API virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) const;

	/*! Find the nearest highest sequence number from the sequence to last provided.
	    \param requested sequence number to start
	    \param last highest sequence
	    \return the nearest sequence number or 0 if not found */
	F8API virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const;

	/*! Persist a generic value.
	    \param key key to store
	    \param what value string
	    \return true on success */
	F8API virtual bool put(const f8String& key, const f8String& what);

	/*! Retrieve a generic persisted value.
	    \param key key to retrieve
	    \param to target value string
	    \return true on success */
	F8API virtual bool get(const f8String& key, f8String& to) const;

	/*! Delete a generic persisted value by specified key.
	    \param key key to delete
	    \return true on success */
	F8API virtual bool del(const f8String& key);
};

#endif // FIX8_HAVE_LIBHIREDIS

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_PERSIST_HPP_

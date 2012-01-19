//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_PERSIST_HPP_
#define _FIX8_PERSIST_HPP_

#include <db_cxx.h>
#include <tbb/concurrent_queue.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Base (ABC) Persister class
class Persister
{
	Persister(const Persister&);
	Persister& operator=(const Persister&);

protected:
	/// Maximum length of persisted FIX message.
	static const size_t MaxMsgLen = 1024;
	bool _opened;

public:
	/// Ctor.
	Persister() : _opened() {}

	/// Dtor.
	virtual ~Persister() {}

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
	    \param callback method it call with each retrieved message
	    \return number of messages retrieved */
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*callback)(const Session::SequencePair& with)) const = 0;

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

	/// Stop the persister thread.
	virtual void stop() {}
};

//-------------------------------------------------------------------------------------------------
class BDBPersister : public Persister
{
	Thread<BDBPersister> _thread;

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

	tbb::concurrent_bounded_queue<KeyDataBuffer> _persist_queue;
	bool write(const KeyDataBuffer& what) { return _persist_queue.try_push (what) == 0; }

public:
	/// Ctor.
	BDBPersister() : _thread(ref(*this)), _dbEnv(0), _db(new Db(&_dbEnv, 0)), _wasCreated() {}
	/// Dtor.
	virtual ~BDBPersister();

	/*! Open existing database or create new database.
	    \param dbDir database environment directory
	    \param dbFname database name
	    \return true on success */
	virtual bool initialise(const f8String& dbDir, const f8String& dbFname);

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
		bool (Session::*)(const Session::SequencePair& with)) const;

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
	void stop() { write(KeyDataBuffer()); _thread.Join(); }

	/*! Persister thread entry point.
	  \return 0 on success */
	int operator()();	// write thread
};

//-------------------------------------------------------------------------------------------------
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
		bool (Session::*)(const Session::SequencePair& with)) const;

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

} // FIX8

#endif // _FIX8_PERSIST_HPP_

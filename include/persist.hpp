//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------------------------
$Id: session.hpp 538 2010-10-31 11:22:40Z davidd $
$Date: 2010-10-31 22:22:40 +1100 (Sun, 31 Oct 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/session.hpp $

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_PERSIST_HPP_
#define _FIX8_PERSIST_HPP_

#include <db_cxx.h>
#include <tbb/concurrent_queue.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class Persister
{
	Persister(const Persister&);
	Persister& operator=(const Persister&);

protected:
	bool _opened;

public:
	Persister() : _opened() {}
	virtual ~Persister() {}

	virtual bool put(const unsigned seqnum, const f8String& what) = 0;
	virtual bool get(const unsigned seqnum, f8String& to) = 0;
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with)) = 0;
	virtual unsigned get_last_seqnum(unsigned& to) const = 0;
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum) = 0;
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum) = 0;
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

	static const size_t MaxMsgLen = 1024;

   struct KeyDataBuffer
   {
      union Ubuf
      {
         unsigned int_;
         char char_[sizeof(unsigned)];
         Ubuf() : int_() {}
         Ubuf(const unsigned val) : int_(val) {}
      }
      keyBuf_;
		unsigned dataBufLen_;
      char dataBuf_[MaxMsgLen];

      KeyDataBuffer() : keyBuf_(), dataBufLen_(), dataBuf_() {}
      KeyDataBuffer(const unsigned ival) : keyBuf_(ival), dataBufLen_(), dataBuf_() {}
      KeyDataBuffer(const unsigned ival, const f8String& src) : keyBuf_(ival), dataBuf_()
			{ src.copy(dataBuf_, dataBufLen_ = src.size() > MaxMsgLen ? MaxMsgLen : src.size()); }
      KeyDataBuffer(const unsigned snd, const unsigned trg) : keyBuf_()
		{
			unsigned *loc(reinterpret_cast<unsigned *>(dataBuf_));
			*loc++ = snd;
			*loc = trg;
		}

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
	BDBPersister() : _thread(ref(*this)), _dbEnv(0), _db(new Db(&_dbEnv, 0)), _wasCreated() {}
	virtual ~BDBPersister();

	virtual bool initialise(const f8String& dbDir, const f8String& dbFname);
	virtual unsigned find_nearest_highest_seqnum (const unsigned requested, const unsigned last);

	virtual bool put(const unsigned seqnum, const f8String& what);
	virtual bool get(const unsigned seqnum, f8String& to);
	virtual unsigned get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*)(const Session::SequencePair& with));
	virtual unsigned get_last_seqnum(unsigned& to) const;
	virtual bool put(const unsigned sender_seqnum, const unsigned target_seqnum);
	virtual bool get(unsigned& sender_seqnum, unsigned& target_seqnum);

	void stop() { write(KeyDataBuffer()); _thread.Join(); }
	int operator()();	// write thread
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_PERSIST_HPP_

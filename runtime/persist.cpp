//-----------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <fix8/f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
extern char glob_log0[max_global_filename_length];

//-------------------------------------------------------------------------------------------------
#if defined FIX8_HAVE_BDB

bool BDBPersister::initialise(const f8String& dbDir, const f8String& dbFname, bool purge)
{
   if (_opened)
      return true;

   _dbDir = dbDir;
   _dbFname = dbFname;

   // Use concurrent db and default shared memory pool
   _dbEnv.open(_dbDir.c_str(), DB_CREATE | DB_INIT_MPOOL | DB_INIT_CDB | DB_THREAD, 0);

	bool notFound(false);

	if (!purge)
	{
		try
		{
			_db->set_bt_compare(bt_compare_fcn);
			_db->open(0, _dbFname.c_str(), 0, DB_BTREE, DB_THREAD, 0); // try and open existing if possible
			unsigned last;
			if (get_last_seqnum(last))
				glout_info << _dbFname << ": Last sequence is " << last;
		}
		catch(DbException& dbe)
		{
			switch (dbe.get_errno())
			{
			case ENOENT:
			case EACCES:
				notFound = true;
				break;
			default:
				glout_error << "Error: opening existing database: " << dbe.what() << " (" << dbe.get_errno() << ')';
				return false;
			}
		}
	}

   if (notFound || purge)  // create a new one
   {
      try
      {
         _db->open(0, _dbFname.c_str(), 0, DB_BTREE, DB_CREATE | DB_THREAD, 0);
			if (purge)
			{
				_db->truncate(0, 0, 0);
				glout_info << "Purged perist db";
			}
      }
      catch(DbException& dbe)
      {
			glout_error << "Error: creating new database: " << dbe.what() << " (" << dbe.get_errno() << ')';
         return false;
      }

      _wasCreated = true;
   }

	_thread.start();
   return _opened = true;

}

//-------------------------------------------------------------------------------------------------
BDBPersister::~BDBPersister()
{
	stop();
	if (_opened)
		_db->close(0);
	delete _db;
	_dbEnv.close(0);

	//cout << "BDBPersister::~BDBPersister()" << endl;
}

//-------------------------------------------------------------------------------------------------
unsigned BDBPersister::get_last_seqnum(unsigned& sequence) const
{
   Dbc *cursorp;
   _db->cursor (0, &cursorp, 0);

   KeyDataBuffer buffer;
   KeyDataPair keyPair(buffer);
   int retval(cursorp->get(&keyPair._key, &keyPair._data, DB_LAST));
   cursorp->close();
   if (retval)
   {
		glout_warn << "last record not found (" << db_strerror(retval) << ')';
      return 0;
   }
   return sequence = buffer.keyBuf_.int_;
}

//-------------------------------------------------------------------------------------------------
unsigned BDBPersister::get(const unsigned from, const unsigned to, Session& session,
	bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(find_nearest_highest_seqnum (from, last_seq));
	const unsigned finish(to == 0 ? last_seq : to);
	Session::RetransmissionContext rctx(from, to, session.get_next_send_seq());

	if (!startSeqNum || from > finish)
	{
		glout_warn << "No records found";
		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
		return 0;
	}

	KeyDataBuffer buffer(startSeqNum);
	KeyDataPair keyPair(buffer);
	Dbc *cursorp;
	_db->cursor (0, &cursorp, 0);
	int retval;

	if ((retval = cursorp->get(&keyPair._key, &keyPair._data, DB_SET)) == 0)
	{
		do
		{
			const unsigned seqnum(buffer.keyBuf_.int_);
			if (!seqnum || seqnum > finish)
				break;
			Session::SequencePair result(seqnum, buffer.dataBuf_);
			++recs_sent;
			if (!(session.*callback)(result, rctx))
				break;
		}
		while(cursorp->get(&keyPair._key, &keyPair._data, DB_NEXT) == 0);

		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
	}
	else
		glout_warn << "record not found (" << db_strerror(retval) << ')';
	cursorp->close();

	return recs_sent;
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::put(const unsigned sender_seqnum, const unsigned target_seqnum)
{
	if (!_opened)
		return false;
	KeyDataBuffer buffer(sender_seqnum, target_seqnum);
	return write(buffer);
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::put(const unsigned seqnum, const f8String& what)
{
	if (!_opened || !seqnum)
		return false;
	KeyDataBuffer buffer(seqnum, what);
	return write(buffer);
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::get(unsigned& sender_seqnum, unsigned& target_seqnum) const
{
	if (!_opened)
      return false;
   KeyDataBuffer buffer(0, 0);
   KeyDataPair keyPair(buffer);
   int retval(_db->get(0, &keyPair._key, &keyPair._data, 0));
   if (retval)
   {
		glout_error << "Could not get control 0 " << '(' << db_strerror(retval) << ')';
      return false;
   }
	unsigned *loc(reinterpret_cast<unsigned *>(buffer.dataBuf_));
	sender_seqnum = *loc++;
	target_seqnum = *loc;
   return true;
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::get(const unsigned seqnum, f8String& to) const
{
	if (!_opened || !seqnum)
      return false;
   KeyDataBuffer buffer(seqnum);
   KeyDataPair keyPair(buffer);
   int retval(_db->get(0, &keyPair._key, &keyPair._data, 0));
   if (retval)
   {
		glout_error << "Could not get " << seqnum << '(' << db_strerror(retval) << ')';
      return false;
   }
   to.assign(buffer.dataBuf_);
   return true;
}

//---------------------------------------------------------------------------------------------------
unsigned BDBPersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const
{
	if (_opened && last)
	{
		for (unsigned startseqnum(requested); startseqnum <= last; ++startseqnum)
		{
			KeyDataBuffer buffer(startseqnum);
			KeyDataPair keyPair(buffer);
			if (_db->get(0, &keyPair._key, &keyPair._data, 0) == 0)
				return startseqnum;
		}
	}

   return 0;
}

//-------------------------------------------------------------------------------------------------
int BDBPersister::operator()()
{
   unsigned received(0), persisted(0);
	bool stopping(false);

   for (;!_cancellation_token;)
   {
		KeyDataBuffer *msg_ptr(0);

#if (FIX8_MPMC_SYSTEM == FIX8_MPMC_TBB)
		KeyDataBuffer buffer;
		if (stopping)	// make sure we dequeue any pending msgs before exiting
		{
			if (!_persist_queue.try_pop(buffer))
				break;
		}
		else
			_persist_queue.pop (buffer); // will block
		msg_ptr = &buffer;

      if (buffer.empty())  // means exit
		{
         stopping = true;
			continue;
		}
#else
		_persist_queue.pop(msg_ptr); // will block
		if (msg_ptr->empty())  // means exit
			break;
#endif
		//cout << "persisted..." << endl;

		++received;

		if (msg_ptr)
		{
			KeyDataPair keyPair(*msg_ptr);
			int retval(_db->put(0, &keyPair._key, &keyPair._data, 0));  // will overwrite if found
			if (retval)
				glout_error << "Could not add" << '(' << db_strerror(retval) << ')';
			else
				++persisted;
		}
#if (FIX8_MPMC_SYSTEM == FIX8_MPMC_FF)
		_persist_queue.release(msg_ptr);
#endif
	}

	//cout << "persister()() exited..." << endl;

	glout_info << received << " messages received, " << persisted << " messages persisted";

   return 0;
}

#endif // FIX8_HAVE_BDB

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
unsigned MemoryPersister::get(const unsigned from, const unsigned to, Session& session,
	bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(find_nearest_highest_seqnum (from, last_seq));
	const unsigned finish(to == 0 ? last_seq : to);
	Session::RetransmissionContext rctx(from, to, session.get_next_send_seq());

	if (!startSeqNum || from > finish)
	{
		glout_warn << "No records found";
		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
		return 0;
	}

	Store::const_iterator itr(_store.find(startSeqNum));
	if (itr != _store.end())
	{
		do
		{
			if (!itr->first || itr->first > finish)
				break;
			Session::SequencePair result(itr->first, itr->second);
			++recs_sent;
			if (!(session.*callback)(result, rctx))
				break;
		}
		while(++itr != _store.end());

		Session::SequencePair result(0, "");
		rctx._no_more_records = true;
		(session.*callback)(result, rctx);
	}
	else
		glout_error << "record not found (" << startSeqNum << ')';

	return recs_sent;
}

//-------------------------------------------------------------------------------------------------
bool MemoryPersister::put(const unsigned sender_seqnum, const unsigned target_seqnum)
{
	const unsigned arr[2] { sender_seqnum, target_seqnum };
	return _store.insert({0, f8String(reinterpret_cast<const char *>(arr), sizeof(arr))}).second;
}

//-------------------------------------------------------------------------------------------------
bool MemoryPersister::put(const unsigned seqnum, const f8String& what)
{
	return !seqnum ? false : _store.insert({seqnum, what}).second;
}

//-------------------------------------------------------------------------------------------------
bool MemoryPersister::get(unsigned& sender_seqnum, unsigned& target_seqnum) const
{
	Store::const_iterator itr(_store.find(0));
	if (itr == _store.end())
		return false;
	const unsigned *loc(reinterpret_cast<const unsigned *>(&itr->second));
	sender_seqnum = *loc++;
	target_seqnum = *loc;
   return true;
}

//-------------------------------------------------------------------------------------------------
bool MemoryPersister::get(const unsigned seqnum, f8String& to) const
{
	if (!seqnum)
		return false;
	Store::const_iterator itr(_store.find(seqnum));
	if (itr == _store.end())
		return false;
	to = itr->second;
   return true;
}

//---------------------------------------------------------------------------------------------------
unsigned MemoryPersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const
{
	if (last)
	{
		for (unsigned startseqnum(requested); startseqnum <= last; ++startseqnum)
		{
			Store::const_iterator itr(_store.find(startseqnum));
			if (itr != _store.end())
				return itr->first;
		}
	}

   return 0;
}

//---------------------------------------------------------------------------------------------------
unsigned MemoryPersister::get_last_seqnum(unsigned& to) const
{
	return to = (_store.empty() ? 0 : _store.rbegin()->first);
}

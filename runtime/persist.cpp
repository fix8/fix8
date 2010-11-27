//-----------------------------------------------------------------------------------------
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

-------------------------------------------------------------------------------------------
$Id: f8cutils.cpp 540 2010-11-05 21:25:33Z davidd $
$Date: 2010-11-06 08:25:33 +1100 (Sat, 06 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/compiler/f8cutils.cpp $

#endif
//-----------------------------------------------------------------------------------------
#include <config.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <cerrno>
#include <regex.h>

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
extern char glob_log0[max_global_filename_length];

//-------------------------------------------------------------------------------------------------
bool BDBPersister::initialise(const f8String& dbDir, const f8String& dbFname)
{
   if (_opened)
      return true;

   _dbDir = dbDir;
   _dbFname = dbFname;

   // Use concurrent db and default shared memory pool
   _dbEnv.open(_dbDir.c_str(), DB_CREATE | DB_INIT_MPOOL | DB_INIT_CDB, 0);

   bool notFound(false);
   try
   {
      _db->set_bt_compare(bt_compare_fcn);
      _db->open(0, _dbFname.c_str(), 0, DB_BTREE, 0, 0); // try and open existing if possible
		unsigned last;
      if (get_last_seqnum(last))
		{
			ostringstream ostr;
         ostr << _dbFname << ": Last sequence is " << last;
			GlobalLogger::instance().send(ostr.str());
		}
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
			{
				ostringstream ostr;
				ostr << "Error opening existing database: " << dbe.what() << " (" << dbe.get_errno() << ')';
				GlobalLogger::instance().send(ostr.str());
			}
         return false;
      }
   }

   if (notFound)  // create a new one
   {
      try
      {
         _db->open(0, _dbFname.c_str(), 0, DB_BTREE, DB_CREATE, 0);
      }
      catch(DbException& dbe)
      {
			ostringstream ostr;
         ostr << "Error creating new database: " << dbe.what() << " (" << dbe.get_errno() << ')';
			GlobalLogger::instance().send(ostr.str());
         return false;
      }

      _wasCreated = true;
   }

	_thread.Start();
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
		ostringstream ostr;
      ostr << "last record not found (" << db_strerror(retval) << ')';
		GlobalLogger::instance().send(ostr.str());
      return 0;
   }
   return sequence = buffer.keyBuf_.int_;
}

//-------------------------------------------------------------------------------------------------
unsigned BDBPersister::get(const unsigned from, const unsigned to, Session& session,
	bool (Session::*callback)(const Session::SequencePair& with))
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(find_nearest_highest_seqnum (from, last_seq));
	const unsigned finish(to == 0 ? last_seq : to);
	if (!startSeqNum || from > finish)
	{
		GlobalLogger::instance().send("No records found");
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
			if (!(session.*callback)(result))
				break;
		}
		while(cursorp->get(&keyPair._key, &keyPair._data, DB_NEXT) == 0);
	}
	else
	{
		ostringstream ostr;
		ostr << "record not found (" << db_strerror(retval) << ')';
		GlobalLogger::instance().send(ostr.str());
	}
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
bool BDBPersister::get(unsigned& sender_seqnum, unsigned& target_seqnum)
{
	if (!_opened)
      return false;
   KeyDataBuffer buffer(0, 0);
   KeyDataPair keyPair(buffer);
   int retval(_db->get(0, &keyPair._key, &keyPair._data, 0));
   if (retval)
   {
		ostringstream ostr;
		ostr << "Could not get control 0 " << '(' << db_strerror(retval) << ')';
		GlobalLogger::instance().send(ostr.str());
      return false;
   }
	unsigned *loc(reinterpret_cast<unsigned *>(buffer.dataBuf_));
	sender_seqnum = *loc++;
	target_seqnum = *loc;
   return true;
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::get(const unsigned seqnum, f8String& to)
{
	if (!_opened || !seqnum)
      return false;
   KeyDataBuffer buffer(seqnum);
   KeyDataPair keyPair(buffer);
   int retval(_db->get(0, &keyPair._key, &keyPair._data, 0));
   if (retval)
   {
		ostringstream ostr;
		ostr << "Could not get " << seqnum << '(' << db_strerror(retval) << ')';
		GlobalLogger::instance().send(ostr.str());
      return false;
   }
   to.assign(buffer.dataBuf_);
   return true;
}

//---------------------------------------------------------------------------------------------------
unsigned BDBPersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last)
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

   for (;;)
   {
		KeyDataBuffer buffer;
		if (stopping)	// make sure we dequeue any pending msgs before exiting
		{
			if (!_persist_queue.try_pop(buffer))
				break;
		}
		else
			_persist_queue.pop (buffer); // will block

      if (buffer.empty())  // means exit
		{
         stopping = true;
			continue;
		}

		++received;

		KeyDataPair keyPair(buffer);
		int retval(_db->put(0, &keyPair._key, &keyPair._data, 0));  // will overwrite if found
		if (retval)
		{
			ostringstream ostr;
			ostr << "Could not add" << '(' << db_strerror(retval) << ')';
			GlobalLogger::instance().send(ostr.str());
		}
		else
			++persisted;
   }

	ostringstream ostr;
	ostr << received << " messages received, " << persisted << " messages persisted";
	GlobalLogger::instance().send(ostr.str());

   return 0;
}


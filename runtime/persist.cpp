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
#include <config.h>

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>
#include <thread.hpp>
#include <session.hpp>
#include <persist.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

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
         cerr << _dbFname << ": Last sequence is " << last << endl;
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
         cerr << "Error opening existing database: " << dbe.what() << " (" << dbe.get_errno() << ')' << endl;
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
         cerr << "Error creating new database: " << dbe.what() << " (" << dbe.get_errno() << ')' << endl;
         return false;
      }

      _wasCreated = true;
   }

   return _opened = true;

}

//-------------------------------------------------------------------------------------------------
BDBPersister::~BDBPersister()
{
	if (_opened)
		_db->close(0);
	delete _db;
	_dbEnv.close(0);
}

//-------------------------------------------------------------------------------------------------
unsigned BDBPersister::get(const unsigned from, const unsigned to,
	bool (Session::*callback)(const Session::SequencePair& with))
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::put(const unsigned seqnum, const f8String& what)
{
	if (!_opened)
		return false;
	KeyDataBuffer buffer(seqnum, what);
	KeyDataPair keyPair(buffer);
	int retval(_db->put(0, &keyPair._key, &keyPair._data, 0));  // will overwrite if found
	if (retval)
	{
		cerr << "Could not add %s(%s) [%s]" << seqnum << db_strerror(retval) << endl;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool BDBPersister::get(const unsigned seqnum, f8String& to)
{
	if (!_opened)
      return false;
   KeyDataBuffer buffer(seqnum);
   KeyDataPair keyPair(buffer);
   int retval(_db->get(0, &keyPair._key, &keyPair._data, 0));
   if (retval)
   {
		cerr << "Could not get %s(%s) [%s]" << seqnum << db_strerror(retval) << endl;
      return false;
   }
   to.assign(buffer.dataBuf_);
   return true;
}

//-------------------------------------------------------------------------------------------------
unsigned BDBPersister::get_last_seqnum(unsigned& to) const
{
	return 0;
}


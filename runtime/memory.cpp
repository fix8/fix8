//-----------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

Copyright (c) 2010-11, David L. Dight <fix@fix8.org>
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

#include <strings.h>
#include <regex.h>
#include <unistd.h>

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
extern char glob_log0[max_global_filename_length];

//-------------------------------------------------------------------------------------------------
MemoryPool::MemoryPool(const size_t numblocks, const size_t blksz) throw(bad_alloc)
   : _store(), _store_end(), _numblocks(numblocks), _blksz(blksz)
{
   _blknum = 0;
   long pageSz;
   if ((pageSz = sysconf(_SC_PAGESIZE)) == -1)
      pageSz = default_page_sz;

   if (posix_memalign(&_store, pageSz * sizeof(void *), _blksz * _numblocks))
      throw bad_alloc();
   _store_end = static_cast<char*>(_store) + _blksz * _numblocks;
   _free_list.set_capacity(_numblocks);
   //cout << "Memorypool allocated " << _blksz * _numblocks << " bytes" << endl;
}

//-------------------------------------------------------------------------------------------------
void *MemoryPool::alloc() throw(bad_alloc)
{
   void *addr;
   if (!_free_list.try_pop(addr))
   {
      addr = static_cast<char *>(_store) + _blksz * _blknum++;
      if (_blknum >= _numblocks)
         throw bad_alloc();
   }
   return addr;
}

//-------------------------------------------------------------------------------------------------
void MemoryPool::reset()
{
	_free_list.clear();
   _blknum = 0;
}

//-------------------------------------------------------------------------------------------------
void MemoryPool::release(void *what) throw(FreelistFull)
{
	if (!allocated(what))
		throw InvalidMemoryPtr(what);
	if (!_free_list.try_push(what))
		throw FreelistFull();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RegionManager::RegionManager(const RegionList& rlist)
{
	for (RegionList::const_iterator itr(rlist.begin()); itr != rlist.end(); ++itr)
		_regions.insert(Regions::value_type(itr->second, new MemoryPool(itr->first, itr->second)));
}

//-------------------------------------------------------------------------------------------------
RegionManager::~RegionManager()
{
	for_each (_regions.begin(), _regions.end(), free_ptr<Delete2ndPairObject<> >());
}

//-------------------------------------------------------------------------------------------------
void RegionManager::reset()
{
	for (Regions::const_iterator itr(_regions.begin()); itr != _regions.end(); ++itr)
		itr->second->reset();
}

//-------------------------------------------------------------------------------------------------
void *RegionManager::alloc(size_t sz) throw(bad_alloc)
{
	Regions::const_iterator itr(_regions.lower_bound(sz));
	return itr == _regions.end() ? malloc(sz) : itr->second->alloc();
}

//-------------------------------------------------------------------------------------------------
void RegionManager::release(void *what) throw(FreelistFull)
{
	for (Regions::const_iterator itr(_regions.begin()); itr != _regions.end(); ++itr)
	{
		if (itr->second->in_range(what))
		{
			itr->second->release(what);
			return;
		}
	}

	free(what);
}

//-------------------------------------------------------------------------------------------------
void RegionManager::report(ostream& os)
{
	unsigned total(0);
	for (Regions::iterator itr(_regions.begin()); itr != _regions.end(); ++itr)
	{
		total += itr->second->get_sz() * itr->second->get_numblocks();
		os << (1 + distance(_regions.begin(), itr)) << ": " << itr->second->get_numblocks()
			<< ' ' << itr->second->get_sz() << ' ' << itr->second->get_blknum() << endl;
	}
	os << "Total memory: " << total << endl;
}

//-------------------------------------------------------------------------------------------------


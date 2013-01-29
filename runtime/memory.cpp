//-----------------------------------------------------------------------------------------
#if 0

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


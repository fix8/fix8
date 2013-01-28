//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_MEMORY_HPP_
#define _FIX8_MEMORY_HPP_

#include <map>
#include <list>
#include <tbb/concurrent_queue.h>
#include <tbb/atomic.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class MemoryPool  // lock free, fixed memory pool
{
   enum { default_page_sz = 4096 };

	tbb::concurrent_bounded_queue<void *> _free_list;
   void *_store, *_store_end;
   const unsigned _numblocks, _blksz;
   tbb::atomic<unsigned> _blknum;

public:
   MemoryPool(const size_t numblocks, const size_t blksz) throw(std::bad_alloc);
   ~MemoryPool() { std::free(_store); }

   void *alloc() throw(std::bad_alloc);
   void release(void *what) throw(FreelistFull);
   void reset();

	unsigned get_blknum() const { return _blknum; }
	unsigned get_sz() const { return _blksz; }
	unsigned get_numblocks() const { return _numblocks; }
	bool in_range(void *what) const { return _store <= what && what < _store_end; }
	bool allocated(void *what) const { return what < static_cast<char *>(_store) + _blksz * _blknum; }
};

//-------------------------------------------------------------------------------------------------
typedef std::pair<unsigned, unsigned> Region;
typedef std::list<Region> RegionList;

class RegionManager
{
	typedef std::map<unsigned, MemoryPool *> Regions;
	Regions _regions;

public:
	RegionManager(const RegionList& rlist);
	virtual ~RegionManager();

	void *alloc(size_t sz) throw(std::bad_alloc);
	void release(void *what) throw(FreelistFull);
	void reset();

	void report(std::ostream& os);
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_MEMORY_HPP_

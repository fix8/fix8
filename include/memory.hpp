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
$Id: session.hpp 554 2010-12-27 11:57:13Z davidd $
$Date: 2010-12-27 22:57:13 +1100 (Sat, 27 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/session.hpp $

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
   static const unsigned default_page_sz = 4096;

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

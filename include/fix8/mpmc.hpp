//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-14 David L. Dight <fix@fix8.org>

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
#ifndef FIX8_MPMC_HPP_
#define FIX8_MPMC_HPP_

//-------------------------------------------------------------------------------------------------
// provide generic names to Multi Producer Multi Consumer queues, mutexes and atomic from
// different libraries

//-------------------------------------------------------------------------------------------------
#if (MPMC_SYSTEM == MPMC_TBB)

# include <tbb/concurrent_queue.h>
# include <tbb/atomic.h>
# include <tbb/mutex.h>

template<typename T> using f8_atomic = tbb::atomic<T>;
using f8_mutex = tbb::mutex;
using f8_spin_lock = tbb::spin_mutex;
template<typename T> using f8_concurrent_queue = tbb::concurrent_bounded_queue<T>;

//-------------------------------------------------------------------------------------------------
#elif (MPMC_SYSTEM == MPMC_FF)

# include <ff/atomic/atomic.h>
# include <ff/allocator.hpp>
# include <ff/buffer.hpp>
# include <ff/MPMCqueues.hpp>
# include <sched.h>

// std wrappers for ff
# include <ff_wrapper.hpp>

template<typename T> using f8_atomic = FIX8::ff_atomic<T>;
template<typename T> using f8_concurrent_queue = FIX8::ff_unbounded_queue<T>;

//-------------------------------------------------------------------------------------------------
#endif // MPMC_SYSTEM

//----------------------------------------------------------------------------------------
/// Your bog standard RAII scoped lock
namespace FIX8 {

template<typename T>
class f8_scoped_lock_impl
{
	T *_local_mutex = nullptr;
	bool _disabled = false;

public:
	f8_scoped_lock_impl() = default;
	f8_scoped_lock_impl(T& mutex) { acquire(mutex); }
	f8_scoped_lock_impl(T& mutex, bool disable) : _disabled(disable)
	{
		if (!_disabled)
			acquire(mutex);
	}

	~f8_scoped_lock_impl() { release(); }

	f8_scoped_lock_impl(const f8_scoped_lock_impl&) = delete;
	f8_scoped_lock_impl& operator=(const f8_scoped_lock_impl&) = delete;

	void acquire(T& mutex)
	{
		mutex.lock();
		_local_mutex = &mutex;
	}

	bool try_acquire(T& mutex)
	{
		bool result(mutex.try_lock());
		if(result)
			_local_mutex = &mutex;
		return result;
	}

	void release()
	{
		if (!_disabled && _local_mutex)
		{
			_local_mutex->unlock();
			_local_mutex = nullptr;
		}
	}
};

using f8_scoped_lock = f8_scoped_lock_impl<f8_mutex>;
using f8_scoped_spin_lock = f8_scoped_lock_impl<f8_spin_lock>;

} // namespace

//-------------------------------------------------------------------------------------------------

#endif // _FIX8_MPMC_HPP_

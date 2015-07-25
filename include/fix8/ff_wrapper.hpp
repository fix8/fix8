//-------------------------------------------------------------------------------------------------
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
#ifndef FIX8_FF_WRAPPER_HPP_
#define FIX8_FF_WRAPPER_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//----------------------------------------------------------------------------------------
template<typename T>
class ff_unbounded_queue
{
	// we could also use ff::MSqueue
	ff::uMPMC_Ptr_Queue _queue;

public:
	using value_type = T;

	//! Reference type
	using reference = T&;

	//! Const reference type
	using const_reference = const T&;

	explicit ff_unbounded_queue() { _queue.init(); }
	~ff_unbounded_queue() {}

	bool try_push(const T& source)
		{ return _queue.push(new (::ff::ff_malloc(sizeof(T))) T(source)); }
	void push(const T& source) { try_push(source); }
	bool try_pop(T* &target) { return _queue.pop(reinterpret_cast<void**>(&target)); }
	bool pop(T* &target)
	{
#if defined FIX8_SLEEP_NO_YIELD
		const unsigned cnt_rnd(3);
		unsigned cnt(0);
#endif
		for(;;)
		{
			if (try_pop(target))
				return true;
#if defined FIX8_SLEEP_NO_YIELD
			if ((++cnt %= cnt_rnd) == 0)
				hypersleep<h_nanoseconds>(FIX8_SLEEP_NO_YIELD);
			else
#endif
				sched_yield();
		}

		return false;
	}
	void release(T *source) const { ::ff::ff_free(source); }
};

//----------------------------------------------------------------------------------------
template<typename T> // pointer specialisation - treat the pointer version identically and gobble the indirection
class ff_unbounded_queue<T*>
{
	ff::uMPMC_Ptr_Queue _queue;

public:
	using value_type = T;

	//! Reference type
	using reference = T&;

	//! Const reference type
	using const_reference = const T&;

	explicit ff_unbounded_queue() { _queue.init(); }
	~ff_unbounded_queue() {}

	bool try_push(T *source) { return _queue.push(source); }
	void push(T *source) { try_push(source); }
	bool try_pop(T* &target) { return _queue.pop(reinterpret_cast<void**>(&target)); }
	bool pop(T* &target)
	{
#if defined FIX8_SLEEP_NO_YIELD
		const unsigned cnt_rnd(3);
		unsigned cnt(0);
#endif
		for(;;)
		{
			if (try_pop(target))
				return true;
#if defined FIX8_SLEEP_NO_YIELD
			if ((++cnt %= cnt_rnd) == 0)
				hypersleep<h_nanoseconds>(FIX8_SLEEP_NO_YIELD);
			else
#endif
				sched_yield();
		}

		return false;
	}
};

//----------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_FF_WRAPPER_HPP_


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
#ifndef _FIX8_ALLOCATOR_HPP_
#define _FIX8_ALLOCATOR_HPP_

//-------------------------------------------------------------------------------------------------
#include <limits>
#include <memory>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
struct BaseAllocator
{
	static Region _rpairs[];
	static RegionManager _mmgr;
};

}

namespace FIX8 {

template <typename T>
class f8Allocator : private BaseAllocator
{
public:
	typedef T value_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <typename U>
	struct rebind { typedef f8Allocator<U> other; };

	pointer address (reference value) const { return &value; }
	const_pointer address (const_reference value) const { return &value; }

	f8Allocator() throw() {}
	f8Allocator(const f8Allocator&) throw() {}
	template <typename U>
	f8Allocator (const f8Allocator<U>&) throw() {}
	~f8Allocator() throw() {}

	size_type max_size () const throw() { return std::numeric_limits<std::size_t>::max() / sizeof(T); }

	pointer allocate (size_type num, const void *hint=0) throw(std::bad_alloc)
		{ return static_cast<pointer>(_mmgr.alloc(num * sizeof(T))); }
	void construct (pointer p, const T& value) { new (static_cast<void*>(p))T(value); }
	void destroy (pointer p) { p->~T(); }
	void deallocate (pointer p, size_type num) throw(std::bad_alloc)
		{ _mmgr.release(static_cast<void*>(p)); }
};

template <typename T1, typename T2>
inline bool operator== (const f8Allocator<T1>&, const f8Allocator<T2>&) throw() { return true; }
template <typename T1, typename T2>
inline bool operator!= (const f8Allocator<T1>&, const f8Allocator<T2>&) throw() { return false; }

}

#endif // _FIX8_ALLOCATOR_HPP_

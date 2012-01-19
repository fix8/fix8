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
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

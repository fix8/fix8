//-------------------------------------------------------------------------------------------------
/*

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
#ifndef _FIX8_FF_WRAPPER_HPP_
# define _FIX8_FF_WRAPPER_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//----------------------------------------------------------------------------------------
template<typename T>
class ff_unbounded_queue
{
	// we could also use ff::MSqueue
	ff::uMPMC_Ptr_Queue _queue;

public:
	typedef T value_type;

	//! Reference type
	typedef T& reference;

	//! Const reference type
	typedef const T& const_reference;

	explicit ff_unbounded_queue() { _queue.init(); }
	~ff_unbounded_queue() {}

	bool try_push(const T& source)
		{ return _queue.push(new (::ff::ff_malloc(sizeof(T))) T(source)); }
	void push(const T& source) { try_push(source); }
	bool try_pop(T* &target) { return _queue.pop(reinterpret_cast<void**>(&target)); }
	bool pop(T* &target)
	{
#if defined SLEEP_NO_YIELD
		const unsigned cnt_rnd(3);
		unsigned cnt(0);
#endif
		for(;;)
		{
			if (try_pop(target))
				return true;
#if defined SLEEP_NO_YIELD
			if ((++cnt %= cnt_rnd) == 0)
				hypersleep<h_nanoseconds>(SLEEP_NO_YIELD);
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
	typedef T value_type;

	//! Reference type
	typedef T &reference;

	//! Const reference type
	typedef const T &const_reference;

	explicit ff_unbounded_queue() { _queue.init(); }
	~ff_unbounded_queue() {}

	bool try_push(T *source) { return _queue.push(source); }
	void push(T *source) { try_push(source); }
	bool try_pop(T* &target) { return _queue.pop(reinterpret_cast<void**>(&target)); }
	bool pop(T* &target)
	{
#if defined SLEEP_NO_YIELD
		const unsigned cnt_rnd(3);
		unsigned cnt(0);
#endif
		for(;;)
		{
			if (try_pop(target))
				return true;
#if defined SLEEP_NO_YIELD
			if ((++cnt %= cnt_rnd) == 0)
				hypersleep<h_nanoseconds>(SLEEP_NO_YIELD);
			else
#endif
				sched_yield();
		}

		return false;
	}
};

//----------------------------------------------------------------------------------------
// C++ wrapper around fastflow atomic
template<typename T>
class ff_atomic
{
	mutable atomic_long_t _rep;

public:
	typedef T value_type;

	value_type operator=(const value_type rhs)
		{ atomic_long_set(&_rep, rhs); return rhs; }

	ff_atomic<value_type>& operator=(const ff_atomic<value_type>& rhs)
		{ atomic_long_set(&_rep, atomic_long_read(&rhs._rep)); return *this; }

	value_type operator++(int) // postfix
		{ value_type v(atomic_long_read(&_rep)); atomic_long_inc(&_rep); return v; }
	value_type operator--(int) // postfix
		{ value_type v(atomic_long_read(&_rep)); atomic_long_dec(&_rep); return v; }
	value_type operator++() // prefix
		{ return atomic_long_inc_return(&_rep); }
	value_type operator--() // prefix
		{ return atomic_long_dec_return(&_rep); }
	value_type operator+=(value_type value)
		{ atomic_long_add(value, &_rep); return atomic_long_read(&_rep); }
	value_type operator-=(value_type value)
		{ atomic_long_sub(value, &_rep); return atomic_long_read(&_rep); }

	operator value_type() { return static_cast<T>(atomic_long_read(&_rep)); }
	operator value_type() const { return static_cast<T>(atomic_long_read(&_rep)); }
};

//----------------------------------------------------------------------------------------
// pointer specialisation of above
template<typename T>
class ff_atomic<T*>
{
	atomic_long_t _rep;

public:
	typedef T* value_type;

	T* operator=(T* rhs)
	{
		atomic_long_set(&_rep, reinterpret_cast<long>(rhs));
		return reinterpret_cast<T*>(atomic_long_read(&_rep));
	}

	ff_atomic<T*>& operator=(const ff_atomic<T*>& rhs)
		{ atomic_long_set(&_rep, atomic_long_read(rhs._rep)); return *this; }

	T* operator->() const { return reinterpret_cast<T*>(atomic_long_read(&_rep)); }
	T operator*() { return *reinterpret_cast<T*>(atomic_long_read(&_rep)); }

	operator value_type() { return reinterpret_cast<T*>(atomic_long_read(&_rep)); }
	operator value_type() const { return reinterpret_cast<T*>(atomic_long_read(&_rep)); }
};

//----------------------------------------------------------------------------------------
/// generic pthread_mutex wrapper
class f8_mutex
{
	pthread_mutex_t _pmutex;

public:
	f8_mutex()
	{
		if (pthread_mutex_init(&_pmutex, 0))
			throw f8Exception("pthread_mutex_init failed");
	}

	~f8_mutex() { pthread_mutex_destroy(&_pmutex); };

	void lock() { pthread_mutex_lock(&_pmutex); }
	bool try_lock() { return pthread_mutex_trylock(&_pmutex) == 0; }
	void unlock() { pthread_mutex_unlock(&_pmutex); }
};

//----------------------------------------------------------------------------------------
/// generic pthread_spin_lock wrapper

#ifdef __APPLE__
// A simple spinlock that spins up to 100K times and then does a sched_yield to back-off.
// unlock() could just set _lock to false BUT that assumes that the spinlock was locked
// to begin with, which may not be the case.  Therefore this implementation has the
// advantage of causing your thread to spin if you try to unlock something that is
// not locked, which I would think is a logic error that the caller should fix.
// The choice of 100K was arbitrary.  The right way to set that parameter would be
// to keep track of how big x gets before thread starvation occurs and use that number.
// That number is going to be target- and use-case-dependent  though.
class f8_spin_lock
{
	bool _isLocked;
public:
	f8_spin_lock() : _isLocked(false) {}
	~f8_spin_lock() {}

	void lock()
	{
		register int x = 0;
		while(!__sync_bool_compare_and_swap(&_isLocked, false, true))
		{
			if(++x >= 100000)
			{
				x = 0;
				sched_yield();
			}
		}
	}
	bool try_lock() { return _isLocked; }
	void unlock()
	{
		register int x = 0;
		while(!__sync_bool_compare_and_swap(&_isLocked, true, false))
		{
			if(++x >= 100000)
			{
				x = 0;
				sched_yield();
			}
		}
	}
};
#else
class f8_spin_lock
{
	pthread_spinlock_t _psl;

public:
	f8_spin_lock()
	{
		if (pthread_spin_init(&_psl, PTHREAD_PROCESS_PRIVATE))
			throw f8Exception("pthread_spin_init failed");
	}

	~f8_spin_lock() { pthread_spin_destroy(&_psl); };

	void lock() { pthread_spin_lock(&_psl); }
	bool try_lock() { return pthread_spin_trylock(&_psl) == 0; }
	void unlock() { pthread_spin_unlock(&_psl); }
};
#endif //__APPLE__

//----------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_FF_WRAPPER_HPP_


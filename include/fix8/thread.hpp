//-----------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------
#ifndef FIX8_THREAD_HPP_
#define FIX8_THREAD_HPP_

//----------------------------------------------------------------------------------------
#include <atomic>
#include <memory>
#if (THREAD_SYSTEM == THREAD_PTHREAD)
#include<pthread.h>
#include<signal.h>
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
#include<thread>
#include<mutex>
#endif

//----------------------------------------------------------------------------------------
namespace FIX8
{

template<typename T> using f8_atomic = std::atomic <T>;

//----------------------------------------------------------------------------------------
/// pthread wrapper abstract base
class _dthreadcore
{
#if (THREAD_SYSTEM == THREAD_STDTHREAD)
public:
	using thread_id_t = std::thread::id;
private:
	std::unique_ptr<std::thread> _thread;
#elif (THREAD_SYSTEM == THREAD_PTHREAD)
public:
	using thread_id_t = pthread_t;
private:
	pthread_attr_t _attr;
	pthread_t _tid;
#endif

#if (THREAD_SYSTEM == THREAD_PTHREAD)
	template<typename T>
	static void *_run(void *what) { return reinterpret_cast<void *>((*static_cast<T *>(what))()); }
#else
	template<typename T>
	static void _run(void *what) { (*static_cast<T *>(what))(); }
#endif

protected:
	int _exitval = 0;

	template<typename T>
	int _start(void *sub)
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_create(&_tid, &_attr, _run<T>, sub);
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
		_thread.reset(new std::thread(_run<T>, sub));
#endif
		return 0;
	}

public:
	/*! Ctor.
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	_dthreadcore(const bool detach, const size_t stacksize)
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		: _attr(), _tid()
#endif
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		if (pthread_attr_init(&_attr))
			throw dthreadException("pthread_attr_init failure");
		if (stacksize && pthread_attr_setstacksize(&_attr, stacksize))
			throw dthreadException("pthread_attr_setstacksize failure");
		if (detach && pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED))
			throw dthreadException("pthread_attr_setdetachstate failure");
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
#endif
	}

	/// Dtor.
	virtual ~_dthreadcore()
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		pthread_attr_destroy(&_attr);
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
#endif
	}

	/*! start thread.
	  \return function result */
	virtual int start() = 0;	// ABC

	/*! request thread stop.
	  \return function result */
	virtual void request_stop() = 0;

	/*! Join the thread.
	  \return result of join */
	virtual int join(int timeoutInMs = 0)
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		void *rptr(&_exitval);
		return pthread_join(_tid, &rptr) ? -1 : _exitval;
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
		if (_thread.get() && _thread->joinable())
			_thread->join();
		return _exitval;
#endif
	}

	/*! Yield CPU.
		\return result of yield */
#ifndef _MSC_VER
#ifdef __APPLE__
	int yield() const { return sched_yield(); }
#else
	int yield() const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_yield();
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
		std::this_thread::yield();
#endif
		return 0;
	}
#endif
#endif

	/*! Get the thread's thread ID.
	  \return the thread id */
	thread_id_t getdthreadid() const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return _tid;
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
		return _thread.get() ? _thread->get_id() : std::thread::id();
#endif
	}

	/*! Get the thread's thread ID. Static version.
	  \return the thread id */
	static thread_id_t getid()
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_self();
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
		return std::this_thread::get_id();
#endif
	}

	/*! dthread equivalence operator.
	  \param that the other thread id
	  \return true if the threads are equal */
	bool operator==(const _dthreadcore& that) const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_equal(_tid, that._tid);
#else
		return getdthreadid() == that.getdthreadid();
#endif
	}

	/*! dthread inequivalence operator.
	  \param that the other thread id
	  \return true if the threads are unequal */
	bool operator!=(const _dthreadcore& that) const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return !pthread_equal(_tid, that._tid);
#else
		return getdthreadid() != that.getdthreadid();
#endif
	}

	_dthreadcore& operator=(const _dthreadcore&) = delete;
};

//----------------------------------------------------------------------------------------
/// Thread cancellation token
struct dthread_cancellation_token
{
	/// ctor
	dthread_cancellation_token() { _stop_requested = 0; _thread_state = Unknown; }

	/*! check if a stop has been requested
	  \return true if stop requested */
	bool stop_requested() const { return _stop_requested == 1; }

	/// Tell the thread to stop
	void request_stop() { _thread_state = Stopping; _stop_requested = 1; }

	/*! check if a stop has been requested
	  \return true if stop requested */
	operator bool() const { return stop_requested(); }

	/*! check if a stop has been requested
	  \return false if stop requested */
	bool operator!() const  { return !stop_requested(); }

	/// Thread state enumerations
	enum ThreadState { Unknown, Running, Stopping, Stopped };

	/*! Get the current thread state
	  \return thread state enumeration */
	int thread_state() const { return _thread_state; }

	/*! Set the thread state
	  \param state state to set to */
	void thread_state(ThreadState state) { _thread_state = state; }

private:
	f8_atomic<int> _stop_requested, _thread_state;
};

//----------------------------------------------------------------------------------------
/// POSIX pthread wrapper. dthread by pointer to member function.  Ctor provides T instance and specifies ptr to member to call or defaults to operator()
/*! \tparam T class call thread entry functor */
template<typename T>
class dthread : public _dthreadcore
{
	class _helper
	{
		T& _what;
		int (T::*_method)();
		dthread_cancellation_token& (T::*_cancellation_token_method)();

	public:
		_helper(T& what, int (T::*method)(), dthread_cancellation_token& (T::*cancellation_token_method)())
			: _what(what), _method(method), _cancellation_token_method(cancellation_token_method) {}
		int operator()()
		{
			try
			{
				cancellation_token().thread_state(dthread_cancellation_token::Running);
				const int ret((_what.*_method)());
				cancellation_token().thread_state(dthread_cancellation_token::Stopped);
				return ret;
			}
			catch(const std::exception&)
			{
				cancellation_token().thread_state(dthread_cancellation_token::Stopped);
				throw;
			}
		}
		dthread_cancellation_token& cancellation_token() { return (_what.*_cancellation_token_method)(); }
	}
	_sub;

public:
	/*! Ctor. Pointer to object, functor version.
	  \param what instance of class with entry point
	  \param method pointer to entry point method
	  \param cancellation_token_method pointer to cancellation_token
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	dthread(T what, int (T::*method)()=&T::operator(),
		dthread_cancellation_token& (T::*cancellation_token_method)()=&T::cancellation_token, const bool detach=false, const size_t stacksize=0)
		: _dthreadcore(detach, stacksize), _sub(what, method, cancellation_token_method) {}

	/*! Ctor. Reference to object, functor version.
	  \param what reference wrapper of class with entry point
	  \param method reference to entry point method
	  \param cancellation_token_method pointer to cancellation_token
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	dthread(std::reference_wrapper<T> what, int (T::*method)()=&T::operator(),
		dthread_cancellation_token& (T::*cancellation_token_method)()=&T::cancellation_token, const bool detach=false, const size_t stacksize=0)
		: _dthreadcore(detach, stacksize), _sub(what, method, cancellation_token_method) {}

	/// Dtor.
	virtual ~dthread() {}

	/*! start thread.
	  \return function result */
	int start() { return _start<_helper>(static_cast<void *>(&_sub)); }

	/*! request thread stop.
	  \return function result */
	void request_stop() { _sub.cancellation_token().request_stop(); }
};

//----------------------------------------------------------------------------------------
/// generic pthread_mutex wrapper
#if (THREAD_SYSTEM == THREAD_PTHREAD)
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
#endif
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
#if (THREAD_SYSTEM == THREAD_PTHREAD)
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
#elif (THREAD_SYSTEM == THREAD_STDTHREAD)
using f8_mutex = std::mutex;
class f8_spin_lock
{
	std::atomic_flag _sl; // = ATOMIC_FLAG_INIT # does not compile under vs2013

public:
    f8_spin_lock() { _sl.clear(std::memory_order_relaxed); }
	~f8_spin_lock() = default;

	void lock() { while (!try_lock()); }
	bool try_lock() { return !_sl.test_and_set(std::memory_order_acquire); }
	void unlock() { _sl.clear(std::memory_order_release); }
};
#endif
#endif //__APPLE__

template<typename T>
class f8_scoped_lock_impl
{
    T *_local_mutex = nullptr;
    bool _disabled = false;

public:
    f8_scoped_lock_impl() = default;
    f8_scoped_lock_impl( T& mutex ) { acquire( mutex ); }
    f8_scoped_lock_impl( T& mutex, bool disable ) : _disabled( disable )
    {
        if ( !_disabled )
            acquire( mutex );
    }

    ~f8_scoped_lock_impl() { release(); }

    f8_scoped_lock_impl( const f8_scoped_lock_impl& ) = delete;
    f8_scoped_lock_impl& operator=( const f8_scoped_lock_impl& ) = delete;

    void acquire( T& mutex )
    {
        mutex.lock();
        _local_mutex = &mutex;
    }

    bool try_acquire( T& mutex )
    {
        bool result( mutex.try_lock() );
        if ( result )
            _local_mutex = &mutex;
        return result;
    }

    void release()
    {
        if ( !_disabled && _local_mutex )
        {
            _local_mutex->unlock();
            _local_mutex = nullptr;
        }
    }
};

using f8_scoped_lock = f8_scoped_lock_impl<f8_mutex>;
using f8_scoped_spin_lock = f8_scoped_lock_impl<f8_spin_lock>;

} // FIX8

#endif // FIX8_THREAD_HPP_

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
#if (THREAD_SYSTEM == THREAD_PTHREAD)
#include<pthread.h>
#include<signal.h>
#elif (THREAD_SYSTEM == THREAD_POCO)
#include <Poco/Thread.h>
#include <Poco/ThreadTarget.h>
#elif (THREAD_SYSTEM == THREAD_TBB)
#include <tbb/tbb_thread.h>
#endif

//----------------------------------------------------------------------------------------
namespace FIX8
{

//----------------------------------------------------------------------------------------
/// pthread wrapper abstract base
class _dthreadcore
{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
public:
	using thread_id_t = pthread_t;
private:
	pthread_attr_t _attr;
	pthread_t _tid;
#elif (THREAD_SYSTEM == THREAD_POCO)
public:
	using thread_id_t = Poco::Thread::TID;
private:
	Poco::Thread _thread;
#elif (THREAD_SYSTEM == THREAD_TBB)
public:
	using thread_id_t = tbb::tbb_thread::id;
private:
	std::unique_ptr<tbb::tbb_thread> _thread;
#endif

#if (THREAD_SYSTEM == THREAD_PTHREAD)
	template<typename T>
	static void *_run(void *what) { return reinterpret_cast<void *>((*static_cast<T *>(what))()); }
#elif (THREAD_SYSTEM == THREAD_POCO || THREAD_SYSTEM == THREAD_TBB)
	template<typename T>
	static void _run(void *what) { (*static_cast<T *>(what))(); }
#endif
	_dthreadcore& operator=(const _dthreadcore&);

protected:
	int _exitval = 0;

	template<typename T>
	int _start(void *sub)
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_create(&_tid, &_attr, _run<T>, sub);
#elif (THREAD_SYSTEM == THREAD_POCO)
		_thread.start(_run<T>, sub);
		return 0;
#elif (THREAD_SYSTEM == THREAD_TBB)
		_thread.reset(new tbb::tbb_thread(_run<T>, sub));
		return 0;
#endif
	}

public:
	/*! Ctor.
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	_dthreadcore(const bool detach, const size_t stacksize)
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		: _attr(), _tid()
#elif (THREAD_SYSTEM == THREAD_POCO)
		: _thread()
#endif
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		if (pthread_attr_init(&_attr))
			throw dthreadException("pthread_attr_init failure");
		if (stacksize && pthread_attr_setstacksize(&_attr, stacksize))
			throw dthreadException("pthread_attr_setstacksize failure");
		if (detach && pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED))
			throw dthreadException("pthread_attr_setdetachstate failure");
#elif (THREAD_SYSTEM == THREAD_POCO)
#elif (THREAD_SYSTEM == THREAD_TBB)
#endif
	}

	/// Dtor.
	virtual ~_dthreadcore()
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		pthread_attr_destroy(&_attr);
#elif (THREAD_SYSTEM == THREAD_POCO)
#elif (THREAD_SYSTEM == THREAD_TBB)
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
#elif (THREAD_SYSTEM == THREAD_POCO)
		try
		{
			if (_thread.isRunning())
			{
				if (timeoutInMs == 0)
					_thread.join();
				else
					_thread.join(timeoutInMs);
			}
		}
		catch(const Poco::SystemException& ex)
		{
			// this is due to poco throws exceptions in case of thread was stopped already or not running
			return -1;
		}
		catch(const Poco::TimeoutException& ex)
		{
			// this is due to poco throws exceptions in case of waiting for thread exit timed out
			return -2;
		}
		return _exitval;
#elif (THREAD_SYSTEM == THREAD_TBB)
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
#elif (THREAD_SYSTEM == THREAD_POCO)
		Poco::Thread::yield();
		return 0;
#elif (THREAD_SYSTEM == THREAD_TBB)
		tbb::this_tbb_thread::yield();
		return 0;
#endif
	}
#endif
#endif

	/*! Get the thread's thread ID.
	  \return the thread id */
	thread_id_t getdthreadid() const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return _tid;
#elif (THREAD_SYSTEM == THREAD_POCO)
		return _thread.currentTid();
#elif (THREAD_SYSTEM == THREAD_TBB)
		return _thread.get() ? _thread->get_id() : tbb::tbb_thread::id();
#endif
	}

	/*! Get the thread's thread ID. Static version.
	  \return the thread id */
	static thread_id_t getid()
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_self();
#elif (THREAD_SYSTEM == THREAD_POCO)
		return Poco::Thread::currentTid();
#elif (THREAD_SYSTEM == THREAD_TBB)
		return tbb::this_tbb_thread::get_id();
#endif
	}

	/*! dthread equivalence operator.
	  \param that the other thread id
	  \return true if the threads are equal */
	bool operator==(const _dthreadcore& that) const
	{
#if (THREAD_SYSTEM == THREAD_PTHREAD)
		return pthread_equal(_tid, that._tid);
#elif (THREAD_SYSTEM == THREAD_POCO || THREAD_SYSTEM == THREAD_TBB)
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
#elif (THREAD_SYSTEM == THREAD_POCO || THREAD_SYSTEM == THREAD_TBB)
		return getdthreadid() != that.getdthreadid();
#endif
	}
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

#if (THREAD_SYSTEM == THREAD_POCO)
	/*! Join the thread.
	  \param timeoutInMs optional timeout in ms (Poco only)
	  \return result of join */
	int join(int timeoutInMs=0)
	{
		const int ts(_sub.cancellation_token().thread_state());
		if (ts == dthread_cancellation_token::Stopping || ts == dthread_cancellation_token::Stopped)
			return _exitval;
		return _dthreadcore::join(timeoutInMs);
	}
#endif
};

} // FIX8

#endif // FIX8_THREAD_HPP_

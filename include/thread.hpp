//-----------------------------------------------------------------------------------------
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
#ifndef _FIX8_THREAD_HPP_
# define _FIX8_THREAD_HPP_

//----------------------------------------------------------------------------------------
#include<pthread.h>
#include<signal.h>

//----------------------------------------------------------------------------------------
namespace FIX8
{

//----------------------------------------------------------------------------------------
/// This is a modified and stripped down version of c++11 reference_wrapper.
/*!  \tparam T class to reference wrap */
template<typename T>
class reference_wrapper
{
	T *_data;

public:
	/*! Ctor.
	  \param _indata instance of object to wrapper */
	reference_wrapper(T& _indata) : _data(&_indata) {}

	/*! Cast to enclosed type operator
	  \return reference to object */
	operator T&() const { return this->get(); }

	/*! Accessor.
	  \return reference to object */
	T& get() const { return *_data; }
};

/// Denotes a reference should be taken to a variable.
/*! \tparam T class to wrapper
   \param _t instance of class
   \return reference_wrappered object */
template<typename T>
inline reference_wrapper<T> ref(T& _t) { return reference_wrapper<T>(_t); }

/// Denotes a const reference should be taken to a variable.
/*! \tparam T class to wrapper
    \param _t instance of class
    \return const reference_wrappered object */
template<typename T>
inline reference_wrapper<const T> cref(T& _t) { return reference_wrapper<const T>(_t); }

//----------------------------------------------------------------------------------------
/// pthread wrapper abstract base
class _dthreadcore
{
	pthread_attr_t _attr;
	pthread_t _tid;
	int _exitval;

	template<typename T>
	static void *_run(void *what)
		{ return reinterpret_cast<void *>((*static_cast<T *>(what))()); }

	_dthreadcore& operator=(const _dthreadcore&);

protected:

	template<typename T>
	int _start(void *sub) { return pthread_create(&_tid, &_attr, _run<T>, sub); }

public:
	/*! Ctor.
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	_dthreadcore(const bool detach, const size_t stacksize) : _attr(), _tid(), _exitval()
	{
		if (pthread_attr_init(&_attr))
			throw dthreadException("pthread_attr_init failure");
		if (stacksize && pthread_attr_setstacksize(&_attr, stacksize))
			throw dthreadException("pthread_attr_setstacksize failure");
		if (detach && pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED))
			throw dthreadException("pthread_attr_setdetachstate failure");
	}

	/// Dtor.
	virtual ~_dthreadcore() { pthread_attr_destroy(&_attr); }

	/*! start thread.
	  \return function result */
	virtual int start() = 0;	// ABC

	/*! Join the thread.
	  \return result of join */
	int join()
		{ return pthread_join(_tid, reinterpret_cast<void **>(&_exitval)) ? -1 : _exitval; }

	/*! Cause the thread to exit.
	  \param exitvalue value to return to calling process */
	void exit(int exitvalue) const { pthread_exit(reinterpret_cast<void *>(exitvalue)); }

	/*! Recover the thread's exit value.
	  \return exit value from to calling process */
	int getexitval() const { return _exitval; }

	/*! Yield CPU.
		\return result of yield */
#ifndef _MSC_VER
#ifdef __APPLE__
	int yield() const { return sched_yield(); }
#else
	int yield() const { return pthread_yield(); }
#endif
#endif

	/*! Kill the thread.
	  \param signum signal number to send */
	void kill(int signum) const { pthread_kill(_tid, signum); }

	/*! Kill the thread. Static version.
	  \param ctxt thread context to kill
	  \param signal signal number to send
	  \return true on success */
	static bool kill(_dthreadcore& ctxt, const int signal=SIGTERM)
		{ return pthread_kill(ctxt._tid, signal) == 0; }

	/*! Get the thread's thread ID.
	  \return the thread id */
	pthread_t getdthreadid() const { return _tid; }

	/*! Get the thread's thread ID. Static version.
	  \return the thread id */
	static pthread_t getid() { return pthread_self(); }

	/*! dthread equivalence operator.
	  \param that the other thread id
	  \return true if the threads are equal */
	bool operator==(const _dthreadcore& that) const { return pthread_equal(_tid, that._tid); }

	/*! dthread inequivalence operator.
	  \param that the other thread id
	  \return true if the threads are unequal */
	bool operator!=(const _dthreadcore& that) const { return !pthread_equal(_tid, that._tid); }

#ifndef _MSC_VER
	/*! Set the thread signal mask.
	  \param how block, unblock or mask
	  \param newmask new mask
	  \param oldmask old mask
	  \return true on success */
	static bool setsignalmask(const int how, const sigset_t *newmask, sigset_t *oldmask)
		{ return pthread_sigmask(how, newmask, oldmask)  == 0; }

	/*! Wait for a specified signal.
	  \param set signal set
	  \param sig new mask
	  \return true on success */
	static bool signalwait(const sigset_t *set, int *sig) { return sigwait(set, sig) == 0; }
#endif
};

//----------------------------------------------------------------------------------------
/// POSIX pthread wrapper. dthread by pointer to member function.  Ctor provides T instance and specifies ptr to member to call or defaults to operator()
/*! \tparam T class call thread entry functor */
template<typename T=void *>
class dthread : public _dthreadcore
{
	class _helper
	{
		T& _what;
		int (T::*_method)();

	public:
		_helper(T& what, int (T::*method)()) : _what(what), _method(method) {}
		int operator()() { return (_what.*_method)(); }
	}
	_sub;

public:
	/*! Ctor. Pointer to object, functor version.
	  \param what instance of class with entry point
	  \param method pointer to entry point method
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	dthread(T what, int (T::*method)()=&T::operator(), const bool detach=false, const size_t stacksize=0)
		: _dthreadcore(detach, stacksize), _sub(what, method) {}

	/*! Ctor. Reference to object, functor version.
	  \param what reference wrapper of class with entry point
	  \param method reference to entry point method
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	dthread(reference_wrapper<T> what, int (T::*method)()=&T::operator(), const bool detach=false, const size_t stacksize=0)
		: _dthreadcore(detach, stacksize), _sub(what, method) {}

	/// Dtor.
	virtual ~dthread() {}

	/*! start thread.
	  \return function result */
	int start() { return _start<_helper>(static_cast<void *>(&_sub)); }
};

//----------------------------------------------------------------------------------------
/// POSIX pthread wrapper. Conventional thread started by ptr to non member function with void * args
template<>
class dthread<> : public _dthreadcore
{
	class _helper
	{
		int (*_func)(void *);
		void *_args;

	public:
		_helper(int (*func)(void *), void *args) : _func(func), _args(args) {}
		int operator()() { return (_func)(_args); }
	}
	_sub;

public:
	/*! Ctor.
	  \param func pointer to thread function entry point
	  \param args pointer to function arguments
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	dthread(int (*func)(void *), void *args, const bool detach=false, const size_t stacksize=0)
		: _dthreadcore(detach, stacksize), _sub(func, args) {}

	/// Dtor.
	virtual ~dthread() {}

	/*! start thread.
	  \return function result */
	int start() { return _start<_helper>(static_cast<void *>(&_sub)); }
};

} // FIX8

#endif // _FIX8_THREAD_HPP_

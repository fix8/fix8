//-----------------------------------------------------------------------------------------
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

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-----------------------------------------------------------------------------------------
#ifndef _FIX8_THREAD_HPP_
#define _FIX8_THREAD_HPP_

//----------------------------------------------------------------------------------------
#include<pthread.h>
#include<signal.h>

//----------------------------------------------------------------------------------------
namespace FIX8
{

//----------------------------------------------------------------------------------------
/*! This is a modified and stripped down version of 0x reference_wrapper.
  \tparam T class to reference wrap */
template<typename T>
class reference_wrapper
{
	T *_data;

public:
	/*! Ctor.
	  \param _indata instance of object to wrapper */
	reference_wrapper(T& _indata) : _data(&_indata) {}

	/*! Cast operator
	  \return reference to object */
	operator T&() const { return this->get(); }

	/*! Accessor.
	  \return reference to object */
	T& get() const { return *_data; }
};

/*! Denotes a reference should be taken to a variable.
  \tparam T class to wrapper
  \param _t instance of class
  \return reference_wrappered object */
template<typename T>
inline reference_wrapper<T> ref(T& _t) { return reference_wrapper<T>(_t); }

/*! Denotes a const reference should be taken to a variable.
  \tparam T class to wrapper
  \param _t instance of class
  \return const reference_wrappered object */
template<typename T>
inline reference_wrapper<const T> cref(T& _t) { return reference_wrapper<const T>(_t); }

//----------------------------------------------------------------------------------------
/// Modern POSIX pthread wrapper ABC.
class _threadbase
{
	pthread_attr_t _attr;
	pthread_t _tid;
	int _exitval;

	template<typename T>
	static void *_Run(void *what)
		{ return reinterpret_cast<void *>((*static_cast<T *>(what))()); }

	_threadbase& operator=(const _threadbase&);

protected:

	template<typename T>
	int _Start(void *sub)
		{ return pthread_create(&_tid, &_attr, _Run<T>, sub); }

public:
	/*! Ctor.
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	_threadbase(const bool detach, const size_t stacksize) throw(ThreadException) : _attr(), _tid(), _exitval()
	{
		if (pthread_attr_init(&_attr))
			throw ThreadException("pthread_attr_init failure");
		if (stacksize && pthread_attr_setstacksize(&_attr, stacksize))
			throw ThreadException("pthread_attr_setstacksize failure");
		if (detach && pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED))
			throw ThreadException("pthread_attr_setdetachstate failure");
	}

	/// Dtor.
	virtual ~_threadbase() { pthread_attr_destroy(&_attr); }

	/*! Start thread.
	  \return function result */
	virtual int Start() = 0;	// ABC

	/*! Join the thread.
	  \return result of join */
	int Join()
		{ return pthread_join(_tid, reinterpret_cast<void **>(&_exitval)) ? -1 : _exitval; }

	void Exit(int exitvalue) const { pthread_exit(reinterpret_cast<void *>(exitvalue)); }
	const int GetExitVal() const { return _exitval; }

	void Kill(int signum) const { pthread_kill(_tid, signum); }

	const pthread_t GetThreadID() const { return _tid; }
	static const pthread_t GetID() { return pthread_self(); }

	bool operator==(const _threadbase& that) const { return pthread_equal(_tid, that._tid); }
	bool operator!=(const _threadbase& that) const { return !pthread_equal(_tid, that._tid); }

	static bool Kill(_threadbase& ctxt, const int signal=SIGTERM)
		{ return pthread_kill(ctxt._tid, signal) == 0; }

	static bool SetSignalMask(const int how, const sigset_t *newmask, sigset_t *oldmask)
		{ return pthread_sigmask(how, newmask, oldmask)  == 0; }

	static bool SignalWait(const sigset_t *set, int *sig)
		{ return sigwait(set, sig) == 0; }
};

//----------------------------------------------------------------------------------------
/*! Modern POSIX pthread wrapper.
  thread by pointer to member function
  ctor provides T instance and specifies ptr to member to call or defaults to operator()
  \tparam T class call thread entry functor */
template<typename T=void *>
class Thread : public _threadbase
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
	Thread(T what, int (T::*method)()=&T::operator(), const bool detach=false, const size_t stacksize=0)
		throw(ThreadException) : _threadbase(detach, stacksize), _sub(what, method) {}

	/*! Ctor. Reference to object, functor version.
	  \param what reference wrapper of class with entry point
	  \param method reference to entry point method
	  \param detach detach thread if true
	  \param stacksize default thread stacksize */
	Thread(reference_wrapper<T> what, int (T::*method)()=&T::operator(), const bool detach=false, const size_t stacksize=0)
		throw(ThreadException) : _threadbase(detach, stacksize), _sub(what, method) {}

	/// Dtor.
	virtual ~Thread() {}

	/*! Start thread.
	  \return function result */
	int Start() { return _Start<_helper>(static_cast<void *>(&_sub)); }
};

//----------------------------------------------------------------------------------------
/*! Modern POSIX pthread wrapper.
  conventional thread started by ptr to non member function with void * args */
template<>
class Thread<> : public _threadbase
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
	Thread(int (*func)(void *), void *args, const bool detach=false, const size_t stacksize=0)
		throw(ThreadException) : _threadbase(detach, stacksize), _sub(func, args) {}

	/// Dtor.
	virtual ~Thread() {}

	/*! Start thread.
	  \return function result */
	int Start() { return _Start<_helper>(static_cast<void *>(&_sub)); }
};

} // FIX8

#endif // _FIX8_THREAD_HPP_

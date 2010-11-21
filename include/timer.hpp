//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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
$Id: message.hpp 549 2010-11-14 11:09:12Z davidd $
$Date: 2010-11-14 22:09:12 +1100 (Sun, 14 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/message.hpp $

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_TIMER_HPP_
#define _FIX8_TIMER_HPP_

//-------------------------------------------------------------------------------------------------
#include <queue>
#include <tbb/mutex.h>
#include <sys/time.h>

//----------------------------------------------------------------------------------------
namespace FIX8
{

//---------------------------------------------------------------------------------------------------
class Timespec
{
	timespec _spec;

public:
	Timespec() : _spec() {}
	Timespec(const Timespec& what) : _spec(what._spec) {}
	Timespec(const timespec& what) : _spec(what) {}
   Timespec& operator=(const Timespec& that)
	{
		if (this != &that)
			_spec = that._spec;
		return *this;
	}

	virtual ~Timespec() {}

	operator timespec&() { return _spec; }
	timespec *operator&() { return &_spec; }
	operator void*() { return _spec.tv_sec || _spec.tv_nsec ? this : 0; }
	double AsDouble() const { return _spec.tv_sec + _spec.tv_nsec / Timespec::nanosecD; }

	void clear()
	{
		_spec.tv_sec = 0;
		_spec.tv_nsec = 0;
	}

	bool operator!()
	{
		return !_spec.tv_sec && !_spec.tv_nsec;
	}

	static const int nanosecU = 1000000000;
	static const double nanosecD = 1000000000.;
};

inline timespec operator-(const timespec& newtime, const timespec& oldtime)
{
	timespec result = { newtime.tv_sec - oldtime.tv_sec, newtime.tv_nsec - oldtime.tv_nsec };
	if (result.tv_nsec < 0)
	{
		--result.tv_sec;
		result.tv_nsec += Timespec::nanosecU;
	}

	return result;
}

inline timespec& operator-=(timespec& oldtime, const timespec& newtime)
{
	oldtime.tv_sec -= newtime.tv_sec;
	oldtime.tv_nsec -= newtime.tv_nsec;
	if (oldtime.tv_nsec < 0)
	{
		--oldtime.tv_sec;
		oldtime.tv_nsec += Timespec::nanosecU;
	}

	return oldtime;
}

inline timespec operator+(const timespec& newtime, const timespec& oldtime)
{
	timespec result = { newtime.tv_sec + oldtime.tv_sec, newtime.tv_nsec + oldtime.tv_nsec };
	if (result.tv_nsec >= Timespec::nanosecU)
	{
		++result.tv_sec;
		result.tv_nsec -= Timespec::nanosecU;
	}

	return result;
}

inline timespec& operator+=(timespec& oldtime, const timespec& newtime)
{
	oldtime.tv_sec += newtime.tv_sec;
	oldtime.tv_nsec += newtime.tv_nsec;
	if (oldtime.tv_nsec >= Timespec::nanosecU)
	{
		++oldtime.tv_sec;
		oldtime.tv_nsec -= Timespec::nanosecU;
	}

	return oldtime;
}

inline bool operator==(const timespec& a, const timespec& b)
{
	return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}

inline bool operator!=(const timespec& a, const timespec& b)
{
	return !(a == b);
}

inline bool operator>(const timespec& a, const timespec& b)
{
	return a.tv_sec == b.tv_sec ? a.tv_nsec > b.tv_nsec : a.tv_sec > b.tv_sec;
}

inline bool operator<(const timespec& a, const timespec& b)
{
	return a.tv_sec == b.tv_sec ? a.tv_nsec < b.tv_nsec : a.tv_sec < b.tv_sec;
}

inline bool operator>=(const timespec& a, const timespec& b)
{
	return !(a < b);
}

inline bool operator<=(const timespec& a, const timespec& b)
{
	return !(a > b);
}

//-------------------------------------------------------------------------------------------------
template<typename T>
class Timer;

template<typename T>
class TimerEvent
{
   bool (T::*_callback)();
	mutable Timespec _t;

public:
	TimerEvent(bool (T::*callback)()) : _callback(callback), _t() {}
	~TimerEvent() {}
	void set(const timespec& t) const { _t = t; }
	bool operator<(const TimerEvent<T>& right) const { return FIX8::operator> (_t, right._t); };

	friend class Timer<T>;
};

//-------------------------------------------------------------------------------------------------
template<typename T>
class Timer
{
   T& _monitor;
   Thread<Timer> _thread;
	tbb::mutex _mutex;
   unsigned _granularity;

   std::priority_queue<TimerEvent<T> > _event_queue;

public:
   Timer(T& monitor, int granularity=1) : _monitor(monitor), _thread(ref(*this)), _granularity(granularity) { _thread.Start(); }
   virtual ~Timer() {}

   bool schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS);
   void join() { _thread.Join(); }
   int operator()();
};

//-------------------------------------------------------------------------------------------------
template<typename T>
int Timer<T>::operator()()
{
   const struct timespec tspec = { 0, 1000 * 1000 * _granularity }; // if _granularity == 1 : 1ms at 1000Hz; 10ms at 100Hz
   unsigned elapsed(0);

   while(true)
   {
      bool shouldsleep(false);
      {
			tbb::mutex::scoped_lock guard(_mutex);

         if (_event_queue.size())
         {
            const TimerEvent<T>& op(_event_queue.top());
            if (!op._t) // empty timeval means exit
            {
               _event_queue.pop(); // remove from queue
               break;
            }

            timespec tmp;
				clock_gettime(CLOCK_REALTIME, &tmp);
            if (FIX8::operator<(op._t, tmp))  // has elapsed
            {
					const TimerEvent<T> rop(_event_queue.top());
               _event_queue.pop(); // remove from queue
					guard.release();
					++elapsed;
               if (!(_monitor.*rop._callback)())
						break;
            }
            else
               shouldsleep = true;
         }
         else
            shouldsleep = true;
      }	// we want the lock to go out of scope before we sleep

      if (shouldsleep)
         nanosleep(&tspec, 0);
   }

	std::ostringstream ostr;
	ostr << "Terminating Timeout thread (" << elapsed << " elapsed, " << _event_queue.size()
		<< " queued).";
	GlobalLogger::instance()->send(ostr.str());
	return 0;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
bool Timer<T>::schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS)
{
   struct timespec tofire = {};

   if (timeToWaitMS)
   {
      // Calculate time to fire (secs, nsecs)
      tofire.tv_sec = timeToWaitMS / 1000;
      tofire.tv_nsec = (timeToWaitMS % 1000) * 1000 * 1000;
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		tofire += now;
   }

	what.set(tofire);
	tbb::mutex::scoped_lock guard(_mutex);
	_event_queue.push(what);

   return true;
}

} // FIX8

#endif // _FIX8_TIMER_HPP_


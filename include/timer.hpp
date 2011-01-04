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

//-------------------------------------------------------------------------------------------------
namespace FIX8
{

//---------------------------------------------------------------------------------------------------
class Tickval
{
public:
	typedef unsigned long long ticks;
	static const ticks thousand = 1000ULL;
	static const ticks million = thousand * thousand;
	static const ticks billion = thousand * million;

private:
	ticks _value;

public:
	Tickval(bool settonow=false) : _value() { if (settonow) now(); }
	Tickval(const Tickval& from) : _value(from._value) {}
	explicit Tickval(const ticks& from) : _value(from) {}
	explicit Tickval(const timespec& from) : _value(_cvt(from)) {}
	Tickval& operator=(const Tickval& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	Tickval& operator=(const timespec& that)
	{
		_value = _cvt(that);
		return *this;
	}

	const ticks& get_ticks() const { return _value; }

	Tickval& now() { return get_tickval(*this); }

	unsigned secs() const { return static_cast<unsigned>(_value / billion); }
	unsigned msecs() const { return static_cast<unsigned>(_value % million); }
	unsigned nsecs() const { return static_cast<unsigned>(_value % billion); }
	double todouble() const { return static_cast<double>(_value) / billion; }

	static Tickval get_tickval()
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return Tickval(ts);
	}

	static Tickval& get_tickval(Tickval& to)
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return to = ts;
	}

	bool operator!() const { return _value == 0ULL; }
	operator void*() { return _value == 0ULL ? 0 : this; }
	operator unsigned long long() { return _value; }
	operator double() { return todouble(); }

	friend Tickval operator-(const Tickval& newtime, const Tickval& oldtime);
	friend Tickval& operator-=(Tickval& oldtime, const Tickval& newtime);
	friend Tickval operator+(const Tickval& newtime, const Tickval& oldtime);
	friend Tickval& operator+=(Tickval& oldtime, const Tickval& newtime);
	friend bool operator==(const Tickval& a, const Tickval& b);
	friend bool operator!=(const Tickval& a, const Tickval& b);
	friend bool operator>(const Tickval& a, const Tickval& b);
	friend bool operator<(const Tickval& a, const Tickval& b);
	friend bool operator>=(const Tickval& a, const Tickval& b);
	friend bool operator<=(const Tickval& a, const Tickval& b);
	friend Tickval& operator+=(Tickval& oldtime, const ticks& ns);
	friend Tickval& operator-=(Tickval& oldtime, const ticks& ns);

private:
	ticks _cvt(const timespec& from)
	{
		return billion * static_cast<ticks>(from.tv_sec) + static_cast<ticks>(from.tv_nsec);
	}
};

inline Tickval operator-(const Tickval& newtime, const Tickval& oldtime)
{
	return Tickval(newtime._value - oldtime._value);
}

inline Tickval& operator-=(Tickval& oldtime, const Tickval& newtime)
{
	oldtime._value -= newtime._value;
	return oldtime;
}

inline Tickval operator+(const Tickval& newtime, const Tickval& oldtime)
{
	return Tickval(newtime._value + oldtime._value);
}

inline Tickval& operator+=(Tickval& oldtime, const Tickval::ticks& ns)
{
	oldtime._value += ns;
	return oldtime;
}

inline Tickval& operator-=(Tickval& oldtime, const Tickval::ticks& ns)
{
	oldtime._value -= ns;
	return oldtime;
}

inline Tickval& operator+=(Tickval& oldtime, const Tickval& newtime)
{
	oldtime._value += newtime._value;
	return oldtime;
}

inline bool operator==(const Tickval& a, const Tickval& b)
{
	return a._value == b._value;
}

inline bool operator!=(const Tickval& a, const Tickval& b)
{
	return a._value != b._value;
}

inline bool operator>(const Tickval& a, const Tickval& b)
{
	return a._value > b._value;
}

inline bool operator<(const Tickval& a, const Tickval& b)
{
	return a._value < b._value;
}

inline bool operator>=(const Tickval& a, const Tickval& b)
{
	return a._value >= b._value;
}

inline bool operator<=(const Tickval& a, const Tickval& b)
{
	return a._value <= b._value;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
class Timer;

template<typename T>
class TimerEvent
{
   bool (T::*_callback)();
	mutable Tickval _t;

public:
	TimerEvent(bool (T::*callback)()) : _callback(callback), _t() {}
	~TimerEvent() {}
	void set(const Tickval& t) const { _t = t; }
	bool operator<(const TimerEvent<T>& right) const { return _t > right._t; };

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
   Timer(T& monitor, int granularity=1) : _monitor(monitor), _thread(ref(*this)), _granularity(granularity) {}
   virtual ~Timer() {}

   bool schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS);
   void join() { _thread.Join(); }
	void start() { _thread.Start(); }
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

            if (op._t < Tickval::get_tickval())  // has elapsed
            {
					const TimerEvent<T> rop(_event_queue.top()); // take a copy
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
	ostr << "Terminating Timer thread (" << elapsed << " elapsed, " << _event_queue.size()
		<< " queued).";
	GlobalLogger::instance().send(ostr.str());
	return 0;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
bool Timer<T>::schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS)
{
	Tickval tofire;

   if (timeToWaitMS)
   {
      // Calculate time to fire (secs, nsecs)
		Tickval::get_tickval(tofire);
		tofire += timeToWaitMS * Tickval::million;
   }

	what.set(tofire);
	tbb::mutex::scoped_lock guard(_mutex);
	_event_queue.push(what);

   return true;
}

} // FIX8

#endif // _FIX8_TIMER_HPP_


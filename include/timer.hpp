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
#ifndef _FIX8_TIMER_HPP_
#define _FIX8_TIMER_HPP_

//-------------------------------------------------------------------------------------------------
#include <queue>
#include <tbb/mutex.h>
#include <sys/time.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8
{

//-------------------------------------------------------------------------------------------------
template<typename T>
class Timer;

/// Timer event object to provide callback context with Timer.
/*! \tparam T Callback class */
template<typename T>
class TimerEvent
{
   bool (T::*_callback)();
	mutable Tickval _t;

public:
	/*! Ctor.
		The callback method returns a bool. If false, exit timer
	  \param callback pointer to callback method */
	explicit TimerEvent(bool (T::*callback)()) : _callback(callback), _t() {}

	/// Dtor.
	~TimerEvent() {}

	/*! Set the event trigger time.
	 \param t Tickval to assign */
	void set(const Tickval& t) const { _t = t; }

	/*! Less than operator.
	  \param right check if this TimeEvent is less than rhs
	  \return true if less than */
	bool operator<(const TimerEvent<T>& right) const { return _t > right._t; };

	friend class Timer<T>;
};

//-------------------------------------------------------------------------------------------------
/// High resolution timer.
/*! \tparam T callback context class */
template<typename T>
class Timer
{
   T& _monitor;
   Thread<Timer> _thread;
	tbb::mutex _mutex;
   unsigned _granularity;

   std::priority_queue<TimerEvent<T> > _event_queue;

public:
	/*! Ctor.
	  \param monitor reference to callback class
	  \param granularity timer recheck interval in ms */
   explicit Timer(T& monitor, int granularity=1) : _monitor(monitor), _thread(ref(*this)), _granularity(granularity) {}

	/// Dtor.
   virtual ~Timer() {}

	/*! Schedule a timer event. Callback method in event called on timer expiry.
	  \param what TimeEvent to schedule
	  \param timeToWaitMS interval to wait in ms
	  \param hi_res if true, interval to wait is in microsecs
	  \return true on success */
   bool schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS, const bool hi_res=false);

	/*! Kill timer thread.
	  \param sig signal to kill with */
   void kill(const int sig) { _thread.Kill(sig); }

	/// Join timer thread. Wait till exits.
   void join() { _thread.Join(); }

	/// Start the timer thread.
	void start() { _thread.Start(); }

	/*! Timer thread entry point.
	  \return result at timer thread exit */
   int operator()();
};

//-------------------------------------------------------------------------------------------------
template<typename T>
int Timer<T>::operator()()
{
   const struct timespec tspec = { 0, 1000 * 100 * _granularity }; // if _granularity == 1 : 0.1ms at 1000Hz; 1ms at 100Hz
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
			rnanosleep(tspec);
   }

	std::ostringstream ostr;
	ostr << "Terminating Timer thread (" << elapsed << " elapsed, " << _event_queue.size() << " queued).";
	GlobalLogger::instance()->send(ostr.str());
	return 0;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
bool Timer<T>::schedule(const TimerEvent<T>& what, const unsigned timeToWait, const bool hi_res)
{
	Tickval tofire;

   if (timeToWait)
   {
      // Calculate time to fire; hi_res = usecs, lo_res = msecs
		Tickval::get_tickval(tofire);
		tofire += timeToWait * (hi_res ? Tickval::thousand : Tickval::million);
   }

	what.set(tofire);
	tbb::mutex::scoped_lock guard(_mutex);
	_event_queue.push(what);

   return true;
}

//---------------------------------------------------------------------------------------------------
/// High resolution interval timer.
class IntervalTimer
{
   Tickval startTime_, delta_;

public:
	/// Ctor. RAII.
   IntervalTimer() : startTime_(true) {}

	/// Dtor.
   virtual ~IntervalTimer() {}

	/*! Calculate elapsed time (delta).
	  \return reference to this object */
   const IntervalTimer& Calculate()
   {
      Tickval now(true);
      delta_ = now - startTime_;
      return *this;
   }

	/*! Get delta as a double.
	  \return delta as double */
   double AsDouble() const { return delta_.todouble(); }

	/*! Reset the interval start time.
	  \return the old delta as double */
   double Reset()
   {
      const double curr(AsDouble());
		startTime_.now();
      return curr;
   }

	/*! Format the interval to 9 decimal places, insert into stream.
	  \param os the stream to insert into
	  \param what the IntervalTimer object
	  \return the stream, os */
   friend std::ostream& operator<<(std::ostream& os, const IntervalTimer& what)
   {
      std::ostringstream ostr;
      ostr.setf(std::ios::showpoint);
      ostr.setf(std::ios::fixed);
      ostr << std::setprecision(9) << what.AsDouble();
      return os << ostr.str();
   }
};

} // FIX8

#endif // _FIX8_TIMER_HPP_


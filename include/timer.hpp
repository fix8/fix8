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
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_TIMER_HPP_
# define _FIX8_TIMER_HPP_

//-------------------------------------------------------------------------------------------------
#include <iomanip>
#include <queue>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif

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
   dthread<Timer> _thread;
	f8_spin_lock _spin_lock;
   unsigned _granularity;

   std::priority_queue<TimerEvent<T> > _event_queue;

public:
	/*! Ctor.
	  \param monitor reference to callback class
	  \param granularity timer recheck interval in ms */
   explicit Timer(T& monitor, int granularity=10) : _monitor(monitor), _thread(ref(*this)), _granularity(granularity) {}

	/// Dtor.
   virtual ~Timer() {}

	/*! Schedule a timer event. Callback method in event called on timer expiry.
	  \param what TimeEvent to schedule
	  \param timeToWaitMS interval to wait in ms
	  \return true on success */
   bool schedule(const TimerEvent<T>& what, const unsigned timeToWaitMS);

	/*! Empty the scheduler of any pending timer events.
	  \return number of timer events that were waiting on the queue */
   size_t clear();

	/*! Kill timer thread.
	  \param sig signal to kill with */
   void kill(const int sig=SIGKILL) { _thread.kill(sig); }

	/// Join timer thread. Wait till exits.
   void join() { _thread.join(); }

	/// Start the timer thread.
	void start() { _thread.start(); }

	/*! Timer thread entry point.
	  \return result at timer thread exit */
   int operator()();
};

//-------------------------------------------------------------------------------------------------
template<typename T>
int Timer<T>::operator()()
{
   unsigned elapsed(0);

   while(true)
   {
      bool shouldsleep(false);
      {
			f8_scoped_spin_lock guard(_spin_lock);

         if (_event_queue.size())
         {
            const TimerEvent<T>& op(_event_queue.top());
            if (!op._t) // empty timeval means exit
            {
               _event_queue.pop(); // remove from queue
               break;
            }

            if (op._t <= Tickval::get_tickval())  // has elapsed
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
			hypersleep<h_milliseconds>(_granularity);
   }

	std::ostringstream ostr;
	ostr << "Terminating Timer thread (" << elapsed << " elapsed, " << _event_queue.size() << " queued).";
	GlobalLogger::instance()->send(ostr.str());
	return 0;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
size_t Timer<T>::clear()
{
	size_t result(0);
	f8_scoped_spin_lock guard(_spin_lock);

	while (_event_queue.size())
	{
		++result;
		_event_queue.pop(); // remove from queue
	}

	return result;
}

//-------------------------------------------------------------------------------------------------
template<typename T>
bool Timer<T>::schedule(const TimerEvent<T>& what, const unsigned timeToWait)
{
	Tickval tofire;

   if (timeToWait)
   {
      // Calculate time to fire
		Tickval::get_tickval(tofire);
		tofire += timeToWait * Tickval::million;
   }

	what.set(tofire);
	f8_scoped_spin_lock guard(_spin_lock);
	_event_queue.push(what);

   return true;
}

//---------------------------------------------------------------------------------------------------
#if defined __x86_64__ && (defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__))
#define HAVE_RDTSC
inline Tickval::ticks rdtsc()
{
    uint32_t high, low;
    __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
    return (static_cast<Tickval::ticks>(high) << 32) + static_cast<Tickval::ticks>(low);
}
#endif

//---------------------------------------------------------------------------------------------------
/// High resolution interval timer.
class IntervalTimer
{
#if defined USE_RDTSC && defined HAVE_RDTSC
	Tickval::ticks startTime_, delta_;
#else
   Tickval startTime_, delta_;
#endif

public:
	/// Ctor. RAII.
#if defined USE_RDTSC && defined HAVE_RDTSC
   IntervalTimer() : startTime_(rdtsc()) {}
#else
   IntervalTimer() : startTime_(true) {}
#endif

	/// Dtor.
   virtual ~IntervalTimer() {}

	/*! Calculate elapsed time (delta).
	  \return reference to this object */
   const IntervalTimer& Calculate()
   {
#if defined USE_RDTSC && defined HAVE_RDTSC
		Tickval::ticks now(rdtsc());
#else
      Tickval now(true);
#endif
      delta_ = now - startTime_;
      return *this;
   }

	/*! Get delta as a double.
	  \return delta as double */
#if defined USE_RDTSC && defined HAVE_RDTSC
   double AsDouble() const { return delta_; }
#else
   double AsDouble() const { return delta_.todouble(); }
#endif

	/*! Reset the interval start time.
	  \return the old delta as double */
   double Reset()
   {
#if defined USE_RDTSC && defined HAVE_RDTSC
      const Tickval::ticks curr(delta_);
		startTime_ = rdtsc();
#else
      const double curr(AsDouble());
		startTime_.now();
#endif
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
#if defined USE_RDTSC && defined HAVE_RDTSC
      ostr << std::setprecision(9) << what;
#else
      ostr << std::setprecision(9) << what.AsDouble();
#endif
      return os << ostr.str();
   }
};

} // FIX8

#endif // _FIX8_TIMER_HPP_


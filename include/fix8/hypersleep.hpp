//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

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
#ifndef FIX8_HYPERSLEEP_HPP_
#define FIX8_HYPERSLEEP_HPP_

namespace FIX8 {

//----------------------------------------------------------------------------------------
enum hyperunits_t { h_seconds, h_milliseconds, h_microseconds, h_nanoseconds, h_count };

#ifdef _MSC_VER
struct timespec
{
    time_t tv_sec; // seconds
    long tv_nsec;  // nanoseconds
};
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long);
#endif

//----------------------------------------------------------------------------------------
namespace
{
	const unsigned thousand(1000);
	const unsigned million(thousand * thousand);
	const int billion(thousand * million);

#if defined FIX8_HAVE_CLOCK_NANOSLEEP
	inline int execute_clock_nanosleep(timespec ts)
	{
		if (ts.tv_nsec >= billion)
		{
			++ts.tv_sec;
			ts.tv_nsec -= billion;
		}
		return clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0);
	}
#endif
}

//----------------------------------------------------------------------------------------
template<hyperunits_t>
inline int hypersleep (unsigned amt);

//----------------------------------------------------------------------------------------
/*! A more reliable high precision sleep, seconds specialisation
    \param amt amount to sleep
    \return 0 on success */
template<>
inline int hypersleep<h_seconds>(unsigned amt)
{
#if defined FIX8_HAVE_CLOCK_NANOSLEEP
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   ts.tv_sec += amt;
   ts.tv_nsec += (amt % (billion));
	return execute_clock_nanosleep(ts);
#elif defined _MSC_VER
	Sleep(amt * thousand);
	return 0;
#else
	const timespec tspec { amt, amt % billion };
	return nanosleep(&tspec, 0);
#endif
}

//----------------------------------------------------------------------------------------
/*! A more reliable high precision sleep, milliseconds specialisation
    \param amt amount to sleep
    \return 0 on success */
template<>
inline int hypersleep<h_milliseconds>(unsigned amt)
{
#if defined FIX8_HAVE_CLOCK_NANOSLEEP
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   ts.tv_sec += (amt / thousand);
   ts.tv_nsec += (million * (amt % thousand));
	return execute_clock_nanosleep(ts);
#elif defined _MSC_VER
	Sleep(amt);	// milliseconds
	return 0;
#else
	const timespec tspec { amt / thousand, million * (amt % thousand) };
	return nanosleep(&tspec, 0);
#endif
}

//----------------------------------------------------------------------------------------
/*! A more reliable high precision sleep, microseconds specialisation
    \param amt amount to sleep
    \return 0 on success */
template<>
inline int hypersleep<h_microseconds>(unsigned amt)
{
#if defined FIX8_HAVE_CLOCK_NANOSLEEP
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   ts.tv_sec += (amt / million);
   ts.tv_nsec += (thousand * (amt % million));
	return execute_clock_nanosleep(ts);
#elif defined _MSC_VER
	Sleep(amt / million * thousand);
	return 0;
#else
	const timespec tspec { amt / million, thousand * (amt % million) };
	return nanosleep(&tspec, 0);
#endif
}

//----------------------------------------------------------------------------------------
/*! A more reliable high precision sleep, nanoseconds specialisation
    \param amt amount to sleep
    \return 0 on success */
template<>
inline int hypersleep<h_nanoseconds>(unsigned amt)
{
#if defined FIX8_HAVE_CLOCK_NANOSLEEP
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   ts.tv_sec += (amt / billion);
   ts.tv_nsec += amt;
	return execute_clock_nanosleep(ts);
#elif defined _MSC_VER
	Sleep(amt / billion * million);
	return 0;
#else
	const timespec tspec { amt / billion, amt };
	return nanosleep(&tspec, 0);
#endif
}

//----------------------------------------------------------------------------------------
/*! A more reliable high precision sleep
    \param amt amount to sleep
    \param units units that sleep value is in
    \return 0 on success */
inline int hypersleep (unsigned amt, hyperunits_t units)
{
   enum { Div, Mul, Operation };
   static const unsigned hv[h_count][Operation]
   {
      { 1,            billion 	}, // Seconds
      { thousand,     million    }, // Milliseconds
      { million,      thousand   }, // Microseconds
      { billion,   	 1  			}, // Nanoseconds
   };

#if defined FIX8_HAVE_CLOCK_NANOSLEEP
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   ts.tv_sec += (amt / hv[units][Div]);    // calculate time to sleep in secs
   ts.tv_nsec += (hv[units][Mul] * (amt % hv[units][Div]));   // calculate time to sleep in nsecs
	return execute_clock_nanosleep(ts);
#elif defined _MSC_VER
	Sleep(amt);	// milliseconds
	return 0;
#else
	const timespec tspec { amt / hv[units][Div], hv[units][Mul] * (amt % hv[units][Div]) };
	return nanosleep(&tspec, 0);
#endif
}

//----------------------------------------------------------------------------------------
} // namespace FIX8_HYPERSLEEP_HPP_

#endif // FIX8_HYPERSLEEP_HPP_


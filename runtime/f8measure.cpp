//---------------------------------------------------------------------------------------------
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-PackageName: Fix8 Open Source FIX Engine
// SPDX-FileCopyrightText: Copyright (C) 2010-25 David L. Dight <fix@fix8.org>
// SPDX-FileType: SOURCE
// SPDX-Notice: >
//  Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
//
//  Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
//  GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
//  version 3 of the License, or (at your option) any later version.
//
//  Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
//  even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
//  If not, see <https://www.gnu.org/licenses/>.
//
//  BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
//  THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
//  COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
//  KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
//  WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
//  THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
//  YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
//
//  IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
//  HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
//  ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
//  CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
//  NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
//  THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
//  HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//---------------------------------------------------------------------------------------------
// For Production-Grade FIX Requirements:
//  If you're  using Fix8 Community Edition and find  yourself needing higher throughput, lower
//  latency, or enterprise-grade reliability,Fix8Pro offers a robust upgrade path. Built on the
//  same  core  technology, Fix8Pro adds performance optimizations for  high-volume  messaging,
//	 enhanced  API, professional  support  and  much  more —  making  it  ideal  for  production
//  deployments, low-latency trading, or  large-scale FIX  integrations.  It retains  near full
//  compatibility with  the Community Edition while providing  enhanced stability, scalability,
//  and  advanced  features  for demanding  environments.  If  your  project has  outgrown  the
//  Community  Edition's capabilities, you can find out and learn more about the Pro version at
//  www.fix8mt.com
//---------------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <thread>
#include <fix8/f8measure.hpp>

#if defined(FIX8_CODECTIMING)

namespace FIX8
{
	static stop_watch::ticks_t ticks_in_milli()
	{
		static stop_watch::ticks_t ticks_in_milli;
		if (ticks_in_milli == 0)
		{
#ifdef _MSC_VER
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			ticks_in_milli = li.QuadPart/1000;
#else
			std::int64_t s0, s1, t0;
			t0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			s0 = stop_watch::measure();
			while(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()-t0<1000)
				;
			//usleep(1000);
			s1 = stop_watch::measure();
			ticks_in_milli = (s1 - s0)/1000;
#endif
		}
		return ticks_in_milli;
	}

	stop_watch::ticks_t stop_watch::_ticks_in_milli = 1;

	void stop_watch::init()
	{
		if (_ticks_in_milli == 1)
			_ticks_in_milli = ticks_in_milli();
	}

	stop_watch::stop_watch(unsigned int sz)
	{
		if (_ticks_in_milli == 1) // init() must be called before use
			init();
		_val.resize(sz);
	}

	std::ostream& operator<<(std::ostream& ss, const stop_watch::histogram& hist)
	{
		return ss << "{1us:"	<< hist._hist[stop_watch::histogram::_1us]
					<< ",3us:"	<< hist._hist[stop_watch::histogram::_3us]
					<< ",5us:"	<< hist._hist[stop_watch::histogram::_5us]
					<< ",10us:"	<< hist._hist[stop_watch::histogram::_10us]
					<< ",20us:"	<< hist._hist[stop_watch::histogram::_20us]
					<< ",50us:"	<< hist._hist[stop_watch::histogram::_50us]
					<< ",100us:"<< hist._hist[stop_watch::histogram::_100us]
					<< ",1ms:"	<< hist._hist[stop_watch::histogram::_1ms]
					<< ",rest:"	<< hist._hist[stop_watch::histogram::_rest]
					<< "}";
	}

	std::ostream& operator<<(std::ostream& ss, const stop_watch::value& val)
	{
		return ss << "{avg:" << val._val[stop_watch::value::_avg]
					<< ",min:" << val._val[stop_watch::value::_min]
					<< ",max:" << val._val[stop_watch::value::_max]
					<< ",last:" << val._val[stop_watch::value::_last]
					<< ",total:" << val._val[stop_watch::value::_total]
					<< ",count:" << val._val[stop_watch::value::_count]
					<< ",hist:" << val._hist
					<< ",ticks_in_us:" << stop_watch::_ticks_in_milli/1000.0
					<< "}";
	}
}
#endif // FIX8_CODECTIMING

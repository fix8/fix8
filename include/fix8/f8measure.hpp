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
#ifndef FIX8_MEASURE_HPP_
#define FIX8_MEASURE_HPP_

#include <fix8/f8config.h>
#include <fix8/f8dll.h>
#if defined(FIX8_CODECTIMING)

#include <memory.h>
#ifndef _MSC_VER
# include <unistd.h>
#else
# undef min
# undef max
#endif
#include <limits>
#include <vector>
#include <ostream>
#include <chrono>
#include <assert.h>

namespace FIX8
{
	class alignas(16) stop_watch
	{
	public:
		using ticks_t = std::int64_t;
		using value_t = std::int64_t;
		using count_t = std::int64_t;

		struct alignas(16) histogram
		{
			enum  { _1us, _3us, _5us, _10us, _20us, _50us, _100us, _1ms, _rest, _slots };
			count_t _hist[_slots];
			histogram() : _hist() {}
			void count(count_t val)
			{
				static const count_t limits[] { 1, 3, 5, 10, 20, 50, 100, 1000, std::numeric_limits<count_t>::max() };
				for (unsigned ii(0); ii < _rest; ++ii)
				{
					if (val<limits[ii])
					{
						++_hist[ii];
						return;
					}
				}
				++_hist[_rest];
			}
		};

		struct alignas(16) value
		{
			enum ValueMetric { _min, _max, _avg, _last, _total, _count, __max };
			value_t _val[__max];
			histogram _hist;
			value() : _val()
			{
				_val[_min] = std::numeric_limits<value_t>::max();
				_val[_max] = std::numeric_limits<value_t>::min();
			}
			value_t count(value_t val)
			{
				_val[_min] = std::min(val, _val[_min]);
				_val[_max] = std::max(val, _val[_max]);
				_val[_last] = val;
				_val[_total] += val;
				++_val[_count];
				_val[_avg] = _val[_total] / _val[_count]; //(_val[_avg] * (_val[_count]-1) + val) / _val[_count];
				_hist.count(val);
				return val;
			}
			value_t operator[](ValueMetric idx) const { return _val[idx]; }
		};

		std::vector<std::pair<ticks_t, value>> _val;
		F8API static ticks_t _ticks_in_milli;
		F8API static void init();
		F8API explicit stop_watch(unsigned int sz);

#if defined __x86_64__ && (defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__))
#define FIX8_UTILS_HAVE_RDTSC
		static ticks_t rdtsc()
		{
			uint32_t high, low;
			__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
			return (static_cast<ticks_t>(high) << 32) + static_cast<ticks_t>(low);
		}
#endif
#if defined(_MSC_VER)
#define FIX8_UTILS_HAVE_RDTSC
		static ticks_t rdtsc()
		{
			//return __rdtsc();
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return li.QuadPart;
		}
#endif
		static ticks_t measure()
		{
#if defined FIX8_UTILS_HAVE_RDTSC
			return rdtsc()*1000/_ticks_in_milli;
#else
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		}
		ticks_t start(unsigned int idx)
		{
			_val[idx].first = measure();
			return _val[idx].first;
		}
		ticks_t start(unsigned int idx, ticks_t start_measure)
		{
			_val[idx].first = start_measure;
			return start_measure;
		}
		value_t stop(unsigned int idx)
		{
			_val[idx].second.count(static_cast<value_t>(measure() - _val[idx].first));
			return _val[idx].second._val[value::_last];
		}
		value_t stop(unsigned int idx, ticks_t stop_measure)
		{
			_val[idx].second.count(static_cast<value_t>(measure() - stop_measure));
			return _val[idx].second._val[value::_last];
		}
		const value& val(unsigned int idx) const { return _val[idx].second; }
	};

	F8API std::ostream& operator<<(std::ostream& ss, const stop_watch::histogram& hist);
	F8API std::ostream& operator<<(std::ostream& ss, const stop_watch::value& val);
}

#endif // FIX8_CODECTIMING
#endif // FIX8_MEASURE_HPP_

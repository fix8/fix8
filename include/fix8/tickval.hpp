//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_TICKVAL_HPP_
#define FIX8_TICKVAL_HPP_

//-------------------------------------------------------------------------------------------------
#include <chrono>
#ifdef _MSC_VER
#include <limits.h>
#undef min
#undef max
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// High resolution time in nanosecond ticks. Thread safe. Portable, based on std::chrono
/// Assumes we support nanosecond resolution. For implementations that don't, there will be inaccuracies:
/// Windows 7 supports up to 15ms resolution only; for Windows 8 it is 100ns. Linux x86_64 is 1ns.
class Tickval
{
public:
#if defined __APPLE__
	using f8_clock = std::chrono::system_clock;
#else
	using f8_clock = std::chrono::high_resolution_clock;
#endif
	using f8_time_point = std::chrono::time_point<f8_clock>;
	using ticks = decltype(f8_time_point::min().time_since_epoch().count());
	using f8_duration = f8_time_point::duration;
	static const ticks noticks = 0; // this should be a signed value
	static const ticks& errorticks() // 2262-04-12 09:47:16.854775807
	{
		static const auto g_ticks(std::chrono::duration_cast<std::chrono::nanoseconds>
			(f8_time_point::max().time_since_epoch()).count());
		return g_ticks;
	}
	static const ticks thousand = 1000;
	static const ticks million = thousand * thousand;
	static const ticks billion = thousand * million;
	static const ticks second = billion;
	static const ticks minute = 60 * second;
	static const ticks hour = 60 * minute;
	static const ticks day = 24 * hour;
	static const ticks week = 7 * day;

private:
	f8_time_point _value;

public:
	/*! Ctor.
	  \param settonow if true, construct with current time */
	explicit Tickval(bool settonow=false)
		: _value(settonow ? f8_clock::now() : f8_time_point(f8_duration::zero())) {}

	/*! Copy Ctor. */
	Tickval(const Tickval& from) : _value(from._value) {}

	/*! Ctor.
	  \param from construct from raw ticks value (nanoseconds) */
	explicit Tickval(ticks from) : _value(std::chrono::duration_cast<f8_duration>(std::chrono::nanoseconds(from))) {}

	/*! Ctor.
	  \param from construct from time_point value */
	explicit Tickval(f8_time_point from) : _value(from) {}

	/*! Ctor.
	  \param secs seconds
	  \param nsecs nanoseconds */
	Tickval(time_t secs, long nsecs)
		: _value(f8_clock::from_time_t(secs) + std::chrono::duration_cast<f8_duration>(std::chrono::nanoseconds(nsecs))) {}

	/*! Assignment operator. */
	Tickval& operator=(const Tickval& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/*! Assignment operator from ticks
	  \param that ticks to assign from
	  \return *this */
	Tickval& operator=(ticks that)
	{
		_value = f8_time_point(std::chrono::duration_cast<f8_duration>(std::chrono::nanoseconds(that)));
		return *this;
	}

	/*! Get the raw tick value (nanosecs).
	  \return raw ticks */
	ticks get_ticks() const { return std::chrono::duration_cast<std::chrono::nanoseconds>(_value.time_since_epoch()).count(); }

	/*! Assign the current time to this object.
	  \return current object */
	Tickval& now()
	{
		_value = f8_clock::now();
		return *this;
	}

	/*! Get the current number of elapsed seconds
	  \return value in seconds */
	time_t secs() const { return std::chrono::duration_cast<std::chrono::seconds>(_value.time_since_epoch()).count(); }

	/*! Get the current number of elapsed milliseconds (excluding secs)
	  \return value in milliseconds */
	unsigned msecs() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(_value.time_since_epoch()).count() % std::milli::den;
	}

	/*! Get the current number of elapsed microseconds (excluding secs)
	  \return value in microseconds */
	unsigned usecs() const
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(_value.time_since_epoch()).count() % std::micro::den;
	}

	/*! Get the current number of elapsed nanoseconds (excluding secs)
	  \return value in nanoseconds */
	unsigned nsecs() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(_value.time_since_epoch()).count() % std::nano::den;
	}

	/*! Get the current tickval as a double.
	  \return value as a double */
	double todouble() const
	{
		return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(_value.time_since_epoch()).count()) / std::nano::den;
	}

	/*! See if this Tickval holds an error value
	  \return true if an error value */
    bool is_errorval() const { return get_ticks() == errorticks(); }

	/*! See if this Tickval is within the range given
	  \param a lower boundary
	  \param b upper boundary; if an error value, ignore upper range
	  \return true if in range */
	bool in_range(const Tickval& a, const Tickval& b) const
	{
		return !b.is_errorval() ? a <= *this && *this <= b : a <= *this;
	}

	/*! Adjust Tickval by given value
	  \param by amount to adjust; if +ve add, if -ve sub
	  \return adjusted Tickval */
	Tickval& adjust(ticks by) { return *this += by; }

	/*! Generate a Tickval object, constructed with the current time.
	  \return Tickval */
	static Tickval get_tickval() { return Tickval(true); }

	/*! Assign current time to a given Tickval object.
	  \return target Tickval object */
	static Tickval& get_tickval(Tickval& to) { return to = Tickval(true); }

	/*! Get tickval as struct tm
	  \param result ref to struct tm to fill
	  \return ptr to result */
	struct tm *as_tm(struct tm& result) const
	{
		const time_t insecs(std::chrono::system_clock::to_time_t(_value));
#ifdef _MSC_VER
		gmtime_s(&result, &insecs);
#else
		gmtime_r(&insecs, &result);
#endif
		return &result;
	}

	/*! Get tickval as struct tm
	  \return tm structure */
	struct tm get_tm() const
	{
		struct tm result;
		return *as_tm(result);
	}

	/*! Set from secs/nsecs
	  \param secs seconds
	  \param nsecs nanoseconds */
	void set(time_t secs, long nsecs) { _value = Tickval(secs, nsecs)._value; }

	/*! Not operator
	  \return true if no ticks (0) */
	bool operator!() const { return _value.time_since_epoch() == std::chrono::nanoseconds::zero(); }

	/*! Not 0 operator
	  \return true if ticks */
	operator void*() { return _value.time_since_epoch() == std::chrono::nanoseconds::zero() ? nullptr : this; }

	/*! Cast to unsigned long long.
	  \return ticks as unsigned long long */
	operator Tickval::ticks () { return _value.time_since_epoch().count(); }

	/*! Cast to double.
	  \return ticks as double */
	operator double() { return todouble(); }

	/*! Binary negate operator.
	  \param newtime Tickval to subtract from
	  \param oldtime Tickval to subtract
	  \return Result as Tickval */
	friend Tickval operator-(const Tickval& newtime, const Tickval& oldtime)
		{ return Tickval(newtime._value - oldtime._value.time_since_epoch()); }

	/*! Binary negate operator.
	  \param newtime Tickval to subtract from
	  \param oldtime Tickval to subtract
	  \return resulting oldtime */
	friend Tickval& operator-=(Tickval& oldtime, const Tickval& newtime)
	{
		oldtime._value -= newtime._value.time_since_epoch();
		return oldtime;
	}

	/*! Binary addition operator.
	  \param newtime Tickval to add to
	  \param oldtime Tickval to add
	  \return Result as Tickval */
	friend Tickval operator+(const Tickval& newtime, const Tickval& oldtime)
		{ return Tickval(newtime._value + oldtime._value.time_since_epoch()); }

	/*! Binary addition operator.
	  \param newtime Tickval to add to
	  \param oldtime Tickval to add
	  \return resulting oldtime */
	friend Tickval& operator+=(Tickval& oldtime, const Tickval& newtime)
	{
		oldtime._value += newtime._value.time_since_epoch();
		return oldtime;
	}

	/*! Equivalence operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if equal */
	friend bool operator==(const Tickval& a, const Tickval& b) { return a._value == b._value; }

	/*! Inequivalence operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if not equal */
	friend bool operator!=(const Tickval& a, const Tickval& b) { return a._value != b._value; }

	/*! Greater than operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is greater than b */
	friend bool operator>(const Tickval& a, const Tickval& b) { return a._value > b._value; }

	/*! Less than operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is less than b */
	friend bool operator<(const Tickval& a, const Tickval& b) { return a._value < b._value; }

	/*! Greater than or equal to operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is greater than or equal to b */
	friend bool operator>=(const Tickval& a, const Tickval& b) { return a._value >= b._value; }

	/*! Less than or equal to operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is less than or equal to b */
	friend bool operator<=(const Tickval& a, const Tickval& b) { return a._value <= b._value; }

	/*! Binary addition operator.
	  \param oldtime Tickval to add to
	  \param ns ticks to add
	  \return oldtime as Tickval */
	friend Tickval& operator+=(Tickval& oldtime, ticks ns)
	{
		oldtime._value += std::chrono::duration_cast<f8_duration>(std::chrono::nanoseconds(ns));
		return oldtime;
	}

	/*! Binary negate operator.
	  \param oldtime Tickval to subtract from
	  \param ns Tickval to subtract
	  \return oldtime as Tickval */
	friend Tickval& operator-=(Tickval& oldtime, ticks ns)
	{
		oldtime._value -= std::chrono::duration_cast<f8_duration>(std::chrono::nanoseconds(ns));
		return oldtime;
	}

	/*! ostream inserter friend; Tickval printer, localtime
	  \param os stream to insert into
	  \param what Tickval to insert
	  \return ostream object */
	friend std::ostream& operator<<(std::ostream& os, const Tickval& what)
	{
		std::string result;
		return os << GetTimeAsStringMS(result, &what, 9, false);
	}

	/*! ostream inserter friend; Tickval printer, gmtime
	  \param os stream to insert into
	  \param what Tickval to insert
	  \return ostream object */
	friend std::ostream& operator>>(std::ostream& os, const Tickval& what)
	{
		std::string result;
		return os << GetTimeAsStringMS(result, &what, 9, true);
	}
};

} // FIX8

#endif // FIX8_TICKVAL_HPP_

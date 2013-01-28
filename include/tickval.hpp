//-------------------------------------------------------------------------------------------------
#if 0

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

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_TICKVAL_HPP_
#define _FIX8_TICKVAL_HPP_

//-------------------------------------------------------------------------------------------------
#include <sys/time.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8
{

//---------------------------------------------------------------------------------------------------
/// High resolution time in nanosecond ticks. Thread safe.
class Tickval
{
public:
	typedef unsigned long long ticks;
	static const ticks noticks = 0ULL;
	static const ticks thousand = 1000ULL;
	static const ticks million = thousand * thousand;
	static const ticks billion = thousand * million;

private:
	tbb::atomic<ticks> _value;

public:
	/*! Ctor.
	  \param settonow if true, construct with current time */
	Tickval(bool settonow=false) { _value = noticks; if (settonow) now(); }

	/*! Copy Ctor. */
	Tickval(const Tickval& from) { _value = from._value; }

	/*! Ctor.
	  \param from construct from raw ticks value (nanoseconds) */
	explicit Tickval(const ticks& from) { _value = from; }

	/*! Ctor.
	  \param from construct from timespec object */
	explicit Tickval(const timespec& from) { _value = _cvt(from); }

	/*! Assignment operator. */
	Tickval& operator=(const Tickval& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/*! Assignment operator from timespec.
	  \param that timespec object to assign from
	  \return *this */
	Tickval& operator=(const timespec& that)
	{
		_value = _cvt(that);
		return *this;
	}

	/*! Get the raw tick value (nanosecs).
	  \return raw ticks */
	ticks get_ticks() const { return _value; }

	/*! Assign the current time to this object.
	  \return current object */
	Tickval& now() { return get_tickval(*this); }

	/*! Get the current number of elapsed seconds
	  \return value in seconds */
	unsigned secs() const { return static_cast<unsigned>(_value / billion); }

	/*! Get the current number of elapsed milliseconds (excluding secs)
	  \return value in milliseconds */
	unsigned msecs() const { return static_cast<unsigned>(_value % thousand); }

	/*! Get the current number of elapsed microseconds (excluding secs)
	  \return value in microseconds */
	unsigned usecs() const { return static_cast<unsigned>(_value % million); }

	/*! Get the current number of elapsed nanoseconds (excluding secs)
	  \return value in nanoseconds */
	unsigned nsecs() const { return static_cast<unsigned>(_value % billion); }

	/*! Get the current tickval as a double.
	  \return value as a double */
	double todouble() const { return static_cast<double>(_value) / billion; }

	/*! Generate a Tickval object, constructed with the current time.
	  \return Tickval */
	static Tickval get_tickval()
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return Tickval(ts);
	}

	/*! Assign current time to a given Tickval object.
	  \return target Tickval object */
	static Tickval& get_tickval(Tickval& to)
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		return to = ts;
	}

	/*! Not operator
	  \return true if no ticks (0) */
	bool operator!() const { return _value == noticks; }

	/*! Not 0 operator
	  \return true if ticks */
	operator void*() { return _value == noticks ? 0 : this; }

	/*! Cast to unsigned long long.
	  \return ticks as unsigned long long */
	operator unsigned long long() { return _value; }

	/*! Cast to double.
	  \return ticks as double */
	operator double() { return todouble(); }

	/*! Binary negate operator.
	  \param newtime Tickval to subtract from
	  \param oldtime Tickval to subtract
	  \return Result as Tickval */
	friend Tickval operator-(const Tickval& newtime, const Tickval& oldtime);

	/*! Binary negate operator.
	  \param newtime Tickval to subtract from
	  \param oldtime Tickval to subtract
	  \return resulting oldtime */
	friend Tickval& operator-=(Tickval& oldtime, const Tickval& newtime);

	/*! Binary addition operator.
	  \param newtime Tickval to add to
	  \param oldtime Tickval to add
	  \return Result as Tickval */
	friend Tickval operator+(const Tickval& newtime, const Tickval& oldtime);

	/*! Binary addition operator.
	  \param newtime Tickval to add to
	  \param oldtime Tickval to add
	  \return resulting oldtime */
	friend Tickval& operator+=(Tickval& oldtime, const Tickval& newtime);

	/*! Equivalence operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if equal */
	friend bool operator==(const Tickval& a, const Tickval& b);

	/*! Inequivalence operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if not equal */
	friend bool operator!=(const Tickval& a, const Tickval& b);

	/*! Greater than operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is greater than b */
	friend bool operator>(const Tickval& a, const Tickval& b);

	/*! Less than operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is less than b */
	friend bool operator<(const Tickval& a, const Tickval& b);

	/*! Greater than or equal to operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is greater than or equal to b */
	friend bool operator>=(const Tickval& a, const Tickval& b);

	/*! Less than or equal to operator.
	  \param a lhs Tickval
	  \param b rhs Tickval
	  \return true if a is less than or equal to b */
	friend bool operator<=(const Tickval& a, const Tickval& b);

	/*! Binary addition operator.
	  \param oldtime Tickval to add to
	  \param ns ticks to add
	  \return oldtime as Tickval */
	friend Tickval& operator+=(Tickval& oldtime, const ticks& ns);

	/*! Binary negate operator.
	  \param oldtime Tickval to subtract from
	  \param ns Tickval to subtract
	  \return oldtime as Tickval */
	friend Tickval& operator-=(Tickval& oldtime, const ticks& ns);

private:
	/*! Convert timespec to ticks.
	  \param from timespec to convert
	  \return resulting ticks */
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

} // FIX8

#endif // _FIX8_TICKVAL_HPP_


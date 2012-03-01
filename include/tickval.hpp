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
#ifndef _FIX8_TICKVAL_HPP_
#define _FIX8_TICKVAL_HPP_

//-------------------------------------------------------------------------------------------------
#include <sys/time.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8
{

//---------------------------------------------------------------------------------------------------
/// High resolution time in nanosecond ticks.
class Tickval
{
public:
	typedef unsigned long long ticks;
	static const ticks noticks = 0ULL;
	static const ticks thousand = 1000ULL;
	static const ticks million = thousand * thousand;
	static const ticks billion = thousand * million;

private:
	ticks _value;

public:
	/*! Ctor.
	  \param settonow if true, construct with current time */
	Tickval(bool settonow=false) : _value() { if (settonow) now(); }

	/*! Copy Ctor. */
	Tickval(const Tickval& from) : _value(from._value) {}

	/*! Ctor.
	  \param from construct from raw ticks value (nanoseconds) */
	explicit Tickval(const ticks& from) : _value(from) {}

	/*! Ctor.
	  \param from construct from timespec object */
	explicit Tickval(const timespec& from) : _value(_cvt(from)) {}

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
	const ticks& get_ticks() const { return _value; }

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


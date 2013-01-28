//-----------------------------------------------------------------------------------------
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
#ifndef _FIX8_FIELD_HPP_
#define _FIX8_FIELD_HPP_

#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Int2Type idiom. Kudos to Andrei Alexandrescu
/*! \tparam field integer value to make type from */
template<unsigned field>
struct EnumType
{
	enum { val = field };
};

//-------------------------------------------------------------------------------------------------
/// Domain range/set static metadata base class
struct RealmBase
{
	enum RealmType { dt_range, dt_set };

	const void *_range;
	RealmType _dtype;
	FieldTrait::FieldType _ftype;
	const int _sz;
	const char * const *_descriptions;

	/*! Check if this value is a member/in range of the domain set.
	  \tparam T domain type
	  \param what the value to check
	  \return true if in the set or no domain available */
	template<typename T>
	bool is_valid(const T& what) const
	{
		const T *rng(static_cast<const T*>(_range));
		return _dtype == dt_set ? std::binary_search(rng, rng + _sz, what) : *rng <= what && what <= *(rng + 1);
	}

	/*! Get the realm value with the specified index
	  \tparam T domain type
	  \param idx of value
	  \return reference to the associated value */
	template<typename T>
	const T& get_rlm_val(const int idx) const
	{
		return *(static_cast<const T*>(_range) + idx);
	}

	/*! Get the realm index of this value in the domain set.
	  \tparam T domain type
	  \param what the value to check
	  \return the index in the domain set of this value */
	template<typename T>
	int get_rlm_idx(const T& what) const
	{
		if (_dtype == dt_set)
		{
			const T *rng(static_cast<const T*>(_range)), *res(std::lower_bound(rng, rng + _sz, what));
			return res != rng + _sz ? res - rng : -1;
		}
		return 0;
	}
};

//-------------------------------------------------------------------------------------------------
/// The base field class (ABC) for all fields
class BaseField
{
	unsigned short _fnum;

protected:
	const RealmBase *_rlm;

public:
	/*! Ctor.
	  \param fnum field num for this field
	  \param rlm pointer to the realmbase for this field (if available) */
	BaseField(const unsigned short fnum, const RealmBase *rlm=0) : _fnum(fnum), _rlm(rlm) {}

	/// Dtor.
	virtual ~BaseField() {}

	/*! Get the fix tag id of this field.
	  \return fix tag id (field num) */
	unsigned short get_tag() const { return _fnum; }

	/*! Print this field to the supplied stream.
	  \param os stream to print to
	  \return the stream */
	virtual std::ostream& print(std::ostream& os) const = 0;

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	virtual void print(char *to, size_t& sz) const = 0;

	/*! Copy this field.
	  \return the copy */
	virtual BaseField *copy() = 0;

	/*! Get the realm index of this field.
	  \return the realm index */
	virtual int get_rlm_idx() const { return -1; }

	/*! Cast this field to the supplied type.
	  \tparam T target type
	  \return reference to the cast field */
	template<typename T>
	T& from() { return *static_cast<T*>(this); }

	/*! Encode this field to the supplied stream.
	  \param os stream to print to
	  \return the number of bytes encoded */
	size_t encode(std::ostream& os) const
	{
		const std::ios::pos_type where(os.tellp());
		os << _fnum << '=' << *this << default_field_separator;
		return os.tellp() - where;
	}

	/*! Encode this field to the supplied stream.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream
	  \return the number of bytes encoded */
	size_t encode(char *to, size_t& sz) const
	{
		const size_t cur_sz(sz);
		sz += itoa(_fnum, to + sz);
		*(to + sz++) = '=';
		print(to + sz, sz);
		*(to + sz++) = default_field_separator;
		return sz - cur_sz;
	}

	/*! Get the realm pointer for this field.
	  \return the realm pointer */
	const RealmBase *get_realm() const { return _rlm; }

	/*! Inserter friend.
	    \param os stream to send to
	    \param what BaseField reference
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const BaseField& what) { return what.print(os); }
	friend class MessageBase;
};

//-------------------------------------------------------------------------------------------------
/// Field template. There will ONLY be template specialisations of this class using Int2Type idiom.
/*! \tparam T field type
    \tparam field field number (fix tag) */
template<typename T, const unsigned short field>
class Field : public BaseField
{
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for int field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<int, field> : public BaseField
{
protected:
	int _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field), _value() {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const int val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(fast_atoi<int>(from.c_str())) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (int) */
	const int& get() const { return _value; }

	/*! Get field value.
	  \return value (int) */
	const int& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (int) */
	const int& set(const int& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (int) */
	const int& set_from_raw(const f8String& from) { return _value = fast_atoi<int>(from.c_str()); }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const { sz += itoa(_value, to); }
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for f8String field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<f8String, field> : public BaseField
{
protected:
	f8String _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (f8String) */
	const f8String& get() const { return _value; }

	/*! Get field value.
	  \return value (f8String) */
	const f8String& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (f8String) */
	const f8String& set(const f8String& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (f8String) */
	const f8String& set_from_raw(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const { sz += _value.copy(to, _value.size()); }
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for double field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<double, field> : public BaseField
{
protected:
	double _value;
	int _precision;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field), _value(), _precision(2) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field, from._rlm), _value(from._value), _precision(from._precision) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const double& val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val), _precision(2) {}

	/*! Value ctor.
	  \param val value to set
	  \param prec precision digits
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const double& val, const int prec, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val), _precision(prec) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(fast_atof(from.c_str())), _precision(2) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/*! Set the output precision
	  \param prec precision digits */
	void set_precision(const int prec) { _precision = prec; }

	/// Dtor.
	virtual ~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (double) */
	virtual const double& get() const { return _value; }

	/*! Get field value.
	  \return value (double) */
	virtual const double& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (int) */
	virtual const double& set(const double& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (double) */
	virtual const double& set_from_raw(const f8String& from) { return _value = fast_atof(from.c_str()); }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	virtual std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	virtual void print(char *to, size_t& sz) const { sz += std::sprintf(to, "%.*f", _precision, _value); }
#if 0
	virtual void print(char *to, size_t& sz) const
	{
		std::string res;
		PutValue<double>(_value, res);
		res.copy(to, res.size());
		sz += res.size();
	}
#endif
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for unsigned short field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<char, field> : public BaseField
{
	char _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field), _value() {}

	/*! Copy Ctor.
	  \param from field to copy */
	Field (const Field& from) : BaseField(field, from._rlm), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char& val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from[0]) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (char) */
	const char& get() const { return _value; }

	/*! Get field value.
	  \return value (char) */
	const char& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (int) */
	const char& set(const char& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (char`) */
	const char& set_from_raw(const f8String& from) { return _value = from[0]; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const { *to = _value; ++sz; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_MonthYear> MonthYear;

/// Partial specialisation for MonthYear field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<MonthYear, field> : public Field<f8String, field>
{
public:
	/// Ctor.
	Field () : Field<f8String, field>(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<f8String, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_data> data;

/// Partial specialisation for data field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<data, field> : public Field<f8String, field>
{
public:
	/// Ctor.
	Field () : Field<f8String, field>(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<f8String, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimestamp> UTCTimestamp;

/// Partial specialisation for UTCTimestamp field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	static const std::string _fmt_sec, _fmt_ms;
	enum MillisecondIndicator { _sec_only = 17, _with_ms = 21 };
	Poco::DateTime _value;
	int _tzdiff;

protected:
	void format0(short data, char *to, int width) const
	{
		while(width-- > 0)
		{
			to[width] = data % 10 + '0';
			data /= 10;
		}
	}

	void parseDate(std::string::const_iterator &begin, size_t len , short &to) const
	{
		while(len-- > 0)
			to = (to << 3) + (to << 1) + (*begin++ - '0');
	}

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field), _tzdiff() {}

	/*! Copy Ctor.
	  \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value), _tzdiff(from._tzdiff) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const Poco::DateTime& val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field)
	{
		if (from.size() == _sec_only) // 19981231-23:59:59
			DateTimeParse(from, _value, _sec_only);
		else if (from.size() == _with_ms) // 19981231-23:59:59.123
			DateTimeParse(from, _value, _with_ms);
	}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
		{
			_value = that._value;
			_tzdiff = that._tzdiff;
		}
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set
	  \return the new value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Poco::DateTime& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
		{ return os << Poco::DateTimeFormatter::format(_value, _fmt_sec); }

	/*! Format Poco::DateTime into a string.
		 With millisecond, the format string will be "YYYYMMDD-HH:MM:SS.MMM"
		 Without millisecond, the format string will be "YYYYMMDD-HH:MM:SS"
	  \param dateTime input Poco::DateTime object
	  \param to output buffer, should make sure there is enough space reserved
	  \param ind indicating whether need millisecond or not
	  \return length of formatted string */
	size_t DateTimeFormat(const Poco::DateTime& dateTime, char *to, const MillisecondIndicator ind=_sec_only) const;

	/*! Decode a DateTime string into a Poco::DateTime
	  \param from input DateTime string
	  \param dateTime output Poco::DateTime object
	  \param ind indicating whether the string has millisecond or not */
	void DateTimeParse(const std::string& from, Poco::DateTime& dateTime, const MillisecondIndicator ind=_sec_only) const;

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const { sz += DateTimeFormat(_value, to, _with_ms); }
};

template<const unsigned short field>
inline size_t Field<UTCTimestamp, field>::DateTimeFormat(const Poco::DateTime& dateTime, char *to, const MillisecondIndicator ind) const
{
	format0(dateTime.year(), to, 4);
	format0(dateTime.month(), to + 4, 2);
	format0(dateTime.day(), to + 6, 2);
	to[8] = '-';

	format0(dateTime.hour(), to + 9, 2);
	to[11] = ':';

	format0(dateTime.minute(), to + 12, 2);
	to[14] = ':';

	format0(dateTime.second(), to + 15, 2);

	if(ind != _with_ms)
		return _sec_only; //length of "YYYYMMDD-HH:MM:SS"

	to[17] = '.';
	format0(dateTime.millisecond(), to + 18, 3);
	return _with_ms; //length of "YYYYMMDD-HH:MM:SS.MMM"
}

template<const unsigned short field>
inline void Field<UTCTimestamp, field>::DateTimeParse(const std::string& from, Poco::DateTime& dateTime, const MillisecondIndicator ind) const
{
	std::string::const_iterator it(from.begin());
	short year(0), month(0), day(0), hour(0), minute(0), second(0), millisecond(0);

	parseDate(it, 4, year);
	parseDate(it, 2, month);
	parseDate(it, 2, day);
	parseDate(++it, 2, hour);
	parseDate(++it, 2, minute);
	parseDate(++it, 2, second);

	if(ind == _with_ms)
		parseDate(++it, 3, millisecond);

	if (Poco::DateTime::isValid(year, month, day, hour, minute, second, millisecond))
		dateTime.assign(year, month, day, hour, minute, second, millisecond);
}

template<const unsigned short field>
const std::string Field<UTCTimestamp, field>::_fmt_sec("%Y%m%d-%H:%M:%S");
template<const unsigned short field>
const std::string Field<UTCTimestamp, field>::_fmt_ms("%Y%m%d-%H:%M:%S.%i");

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimeOnly> UTCTimeOnly;

/// Partial specialisation for UTCTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const {}  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCDateOnly> UTCDateOnly;

/// Partial specialisation for UTCDateOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCDateOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const {}  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_LocalMktDate> LocalMktDate;

/// Partial specialisation for LocalMktDate field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<LocalMktDate, field> : public BaseField
{
	Poco::DateTime _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const {}  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimeOnly> TZTimeOnly;

/// Partial specialisation for TZTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<TZTimeOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }
	/*! Get field value.
	  \param from value to set
	  \return original value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const {}  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimestamp> TZTimestamp;

/// Partial specialisation for TZTimestamp field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<TZTimestamp, field> : public BaseField
{
	Poco::DateTime _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& get() const { return _value; }

	/*! Get field value.
	  \return value (Poco::DateTime) */
	const Poco::DateTime& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (Poco::DateTime) */
	const Poco::DateTime& set(const f8String& from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const {}  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Length> Length;

/// Partial specialisation for Length field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<Length, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TagNum> TagNum;

/// Partial specialisation for TagNum field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<TagNum, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_SeqNum> SeqNum;

/// Partial specialisation for SeqNum field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<SeqNum, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_NumInGroup> NumInGroup;

/// Partial specialisation for NumInGroup field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<NumInGroup, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_DayOfMonth> DayOfMonth;

/// Partial specialisation for DayOfMonth field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<DayOfMonth, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Boolean> Boolean;

/// Partial specialisation for Boolean field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<Boolean, field> : public BaseField
{
	bool _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(toupper(val) == 'Y') {}

	/*! Value ctor.
	  \param val value to set */
	explicit Field (const bool val) : BaseField(field), _value(val) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(toupper(from[0]) == 'Y') {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Dtor.
	virtual ~Field() {}

	/*! Get field value.
	  \return value (bool) */
	bool get() const { return _value; }

	/*! Get field value.
	  \return value (bool) */
	bool operator()() const { return _value; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value ? 'Y' : 'N') : -1; }

	/*! Get field value.
	  \param from value to set
	  \return original value (bool) */
	bool set(const bool from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (bool) */
	bool set_from_raw(const f8String& from) { return _value = toupper(from[0]) == 'Y'; }

	/*! Copy (clone) this field.
	  \return copy of field */
	virtual Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << (_value ? 'Y' : 'N'); }

	/*! Print this field to the supplied buffer, update size written.
	  \param to buffer to print to
	  \param sz current size of buffer payload stream */
	void print(char *to, size_t& sz) const { *to = _value ? 'Y' : 'N'; ++sz; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_float> Qty;
typedef EnumType<FieldTrait::ft_float> Amt;
typedef EnumType<FieldTrait::ft_float> price;
typedef EnumType<FieldTrait::ft_float> PriceOffset;
typedef EnumType<FieldTrait::ft_float> Percentage;

/// Partial specialisation for Qty field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<Qty, field> : public Field<double, field>
{
public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : Field<double, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const double& val, const RealmBase *rlm=0) : Field<double, field>(val, rlm) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<double, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_string> MultipleCharValue;
typedef EnumType<FieldTrait::ft_string> MultipleStringValue;
typedef EnumType<FieldTrait::ft_string> country;
typedef EnumType<FieldTrait::ft_string> currency;
typedef EnumType<FieldTrait::ft_string> Exchange;
typedef EnumType<FieldTrait::ft_string> Language;
typedef EnumType<FieldTrait::ft_string> XMLData;

/// Partial specialisation for MultipleCharValue field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<MultipleCharValue, field> : public Field<f8String, field>
{
public:
	/*! Value ctor.
	  \param val value to set */
	Field (const f8String& val) : Field<f8String, field>(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm) : Field<f8String, field>(from, rlm) {}

	/// Dtor.
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
/// Field metadata structure
struct BaseEntry
{
	BaseField *(*_create)(const f8String& str, const RealmBase* rlm, const int rv);
	const RealmBase *_rlm;
	const char *_name, *_comment;
};

/*! Construct a BaseEntry object.
  \param be BaseEntry object (target)
  \param create pointer to create function
  \param rlm pointer to the realmbase for this field (if available)
  \param name Field name
  \param comment Field comments
  \return BaseEntry object */
BaseEntry *BaseEntry_ctor(BaseEntry *be, BaseField *(*create)(const f8String&, const RealmBase*, const int),
	const RealmBase *rlm, const char *name, const char *comment);

//-------------------------------------------------------------------------------------------------
// Common (administrative) msgtypes
const f8String Common_MsgType_HEARTBEAT("0");
const f8String Common_MsgType_TEST_REQUEST("1");
const f8String Common_MsgType_RESEND_REQUEST("2");
const f8String Common_MsgType_REJECT("3");
const f8String Common_MsgType_SEQUENCE_RESET("4");
const f8String Common_MsgType_LOGOUT("5");
const f8String Common_MsgType_LOGON("A");

//-------------------------------------------------------------------------------------------------
// Common FIX field numbers

const unsigned Common_BeginSeqNo(7);
const unsigned Common_BeginString(8);
const unsigned Common_BodyLength(9);
const unsigned Common_CheckSum(10);
const unsigned Common_EndSeqNo(16);
const unsigned Common_MsgSeqNum(34);
const unsigned Common_MsgType(35);
const unsigned Common_NewSeqNo(36);
const unsigned Common_PossDupFlag(43);
const unsigned Common_RefSeqNum(45);
const unsigned Common_SenderCompID(49);
const unsigned Common_SendingTime(52);
const unsigned Common_TargetCompID(56);
const unsigned Common_Text(58);
const unsigned Common_EncryptMethod(98);
const unsigned Common_HeartBtInt(108);
const unsigned Common_TestReqID(112);
const unsigned Common_OrigSendingTime(122);
const unsigned Common_GapFillFlag(123);
const unsigned Common_ResetSeqNumFlag(141);
const unsigned Common_DefaultApplVerID(1137);	// >= 5.0 || FIXT1.1

//-------------------------------------------------------------------------------------------------
// Common FIX fields

typedef Field<SeqNum, Common_MsgSeqNum> msg_seq_num;
typedef Field<SeqNum, Common_BeginSeqNo> begin_seq_num;
typedef Field<SeqNum, Common_EndSeqNo> end_seq_num;
typedef Field<SeqNum, Common_NewSeqNo> new_seq_num;
typedef Field<SeqNum, Common_RefSeqNum> ref_seq_num;

typedef Field<Length, Common_BodyLength> body_length;

typedef Field<f8String, Common_SenderCompID> sender_comp_id;
typedef Field<f8String, Common_TargetCompID> target_comp_id;
typedef Field<f8String, Common_MsgType> msg_type;
typedef Field<f8String, Common_CheckSum> check_sum;
typedef Field<f8String, Common_BeginString> begin_string;
typedef Field<f8String, Common_TestReqID> test_request_id;
typedef Field<f8String, Common_Text> text;
typedef Field<f8String, Common_DefaultApplVerID> default_appl_ver_id;

typedef Field<UTCTimestamp, Common_SendingTime> sending_time;
typedef Field<UTCTimestamp, Common_OrigSendingTime> orig_sending_time;

typedef Field<Boolean, Common_GapFillFlag> gap_fill_flag;
typedef Field<Boolean, Common_PossDupFlag> poss_dup_flag;
typedef Field<Boolean, Common_ResetSeqNumFlag> reset_seqnum_flag;

typedef Field<int, Common_HeartBtInt> heartbeat_interval;
typedef Field<int, Common_EncryptMethod> encrypt_method;

} // FIX8

#endif // _FIX8_FIELD_HPP_

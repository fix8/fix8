//-----------------------------------------------------------------------------------------
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
#ifndef _FIX8_FIELD_HPP_
# define _FIX8_FIELD_HPP_

#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
// Misc consts
const size_t MAX_MSGTYPE_FIELD_LEN(32);
const size_t HEADER_CALC_OFFSET(32);

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
	const T& get_rlm_val(const int idx) const { return *(static_cast<const T*>(_range) + idx); }

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
	const unsigned short _fnum;

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

	/*! Print this field to the supplied stream. Used by the Fix8 printer.
	  \param os stream to print to
	  \return the stream */
	virtual std::ostream& print(std::ostream& os) const = 0;

	/*! Print this field to the supplied buffer. Used for encoding.
	  \param to buffer to print to
	  \return number bytes encoded */
	virtual size_t print(char *to) const = 0;

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
	  \param os stream to encode to
	  \return the number of bytes encoded */
	size_t encode(std::ostream& os) const
	{
		const std::ios::pos_type where(os.tellp());
		os << _fnum << default_assignment_separator << *this << default_field_separator;
		return os.tellp() - where;
	}

	/*! Encode this field to the supplied stream. ULL version.
	  \param to buffer to encode to
	  \return the number of bytes encoded */
	size_t encode(char *to) const
	{
		const char *cur_ptr(to);
		to += itoa(_fnum, to, 10);
		*to++ = default_assignment_separator;
		to += print(to);
		*to++ = default_field_separator;
		return to - cur_ptr;
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(fast_atoi<int>(from)) {}

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
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

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
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return itoa(_value, to, 10); }
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for char * field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<char *, field> : public BaseField
{
protected:
	const char *_value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}

	/// Equivalence operator.
	/*! \param that field to compare to
	    \return true if equal */
	bool operator==(const char *that) const { return ::strcmp(_value, that) == 0; }

	/// Inequivalence operator.
	/*! \param that field to compare to
	    \return true if unequal */
	bool operator!=(const char *that) const { return ::strcmp(_value, that); }

	/// Dtor.
	virtual ~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (f8String) */
	const char *get() const { return _value; }

	/*! Get field value.
	  \return value (f8String) */
	const char *operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (f8String) */
	const char *set(const char *from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (f8String) */
	const char *set_from_raw(const char *from) { return _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { ::strcpy(to, _value); return ::strlen(_value); }
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from) {}

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
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

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
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return _value.copy(to, _value.size()); }
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
	Field () : BaseField(field), _value(), _precision(DEFAULT_PRECISION) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field, from._rlm), _value(from._value), _precision(from._precision) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const double& val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val), _precision(DEFAULT_PRECISION) {}

	/*! Value ctor.
	  \param val value to set
	  \param prec precision digits
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const double& val, const int prec, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val), _precision(prec) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(fast_atof(from.c_str())), _precision(DEFAULT_PRECISION) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(fast_atof(from)), _precision(DEFAULT_PRECISION) {}

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
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (double) */
	const double& get() const { return _value; }

	/*! Get field value.
	  \return value (double) */
	const double& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (int) */
	const double& set(const double& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (double) */
	const double& set_from_raw(const f8String& from) { return _value = fast_atof(from.c_str()); }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return modp_dtoa(_value, to, _precision); }
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(*from) {}

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
	~Field() {}

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

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
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << _value; }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { *to = _value; return 1; }
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
inline void format0(int data, char *to, int width)
{
	while(width-- > 0)
	{
		to[width] = data % 10 + '0';
		data /= 10;
	}
}

inline size_t parseDate(const char *begin, size_t len, int &to)
{
	const char *bsv(begin);
	while(len-- > 0)
		to = (to << 3) + (to << 1) + (*begin++ - '0');
	return begin - bsv;
}

/// Based on Ghulam M. Babar's "mktime slow? use custom function"
/// see http://gmbabar.wordpress.com/2010/12/01/mktime-slow-use-custom-function/
inline time_t time_to_epoch (const tm& ltm, int utcdiff=0)
{
   static const int mon_days[] = {0,
      31,
      31 + 28,
      31 + 28 + 31,
      31 + 28 + 31 + 30,
      31 + 28 + 31 + 30 + 31,
      31 + 28 + 31 + 30 + 31 + 30,
      31 + 28 + 31 + 30 + 31 + 30 + 31,
      31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
      31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
      31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
      31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
      31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
   };

   const int tyears(ltm.tm_year ? ltm.tm_year - 70 : 0); // tm->tm_year is from 1900.
   const int tdays(mon_days[ltm.tm_mon] + (ltm.tm_mday ? ltm.tm_mday - 1 : 0) + tyears * 365 + (tyears + 2) / 4);
   return tdays * 86400 + (ltm.tm_hour + utcdiff) * 3600 + ltm.tm_min * 60 + ltm.tm_sec;
}

enum TimeIndicator { _time_only, _time_with_ms, _short_date_only, _date_only, _sec_only, _with_ms };

/*! Format Tickval into a string.
	_time_only, the format string will be "HH:MM:SS"
	_time_with_ms, the format string will be "HH:MM:SS.mmm"
	_short_date_only, the format string will be "YYYYMM"
	_date_only, the format string will be "YYYYMMDD"
	_sec_only, the format string will be "YYYYMMDD-HH:MM:SS"
	_with_ms, the format string will be "YYYYMMDD-HH:MM:SS.mmm"
  \param tickval input Tickval object
  \param to output buffer, should make sure there is enough space reserved
  \param ind indicating whether need millisecond or not
  \return length of formatted string */
inline size_t date_time_format(const Tickval& tickval, char *to, const TimeIndicator ind)
{
   tm result;
	tickval.as_tm(result);
	const char *start(to);

	if (ind > _time_with_ms)
	{
		format0(result.tm_year + 1900, to, 4);
		to += 4;
		format0(result.tm_mon + 1, to, 2);
		to += 2;
		if (ind == _short_date_only)
			return to - start;
		format0(result.tm_mday, to, 2);
		to += 2;
		if (ind == _date_only)
			return to - start;
		*to++ = '-';
	}

	format0(result.tm_hour, to, 2);
	to += 2;
	*to++ = ':';
	format0(result.tm_min, to, 2);
	to += 2;
	*to++ = ':';
	format0(result.tm_sec, to, 2);
	to += 2;

	if (ind == _time_with_ms || ind == _with_ms)
	{
		*to++ = '.';
		format0(tickval.msecs(), to, 3);
		to += 3;
	}

	return to - start;
}

/*! Decode a DateTime string into ticks
  \param ptr input DateTime string
  \param len length of string
  \return ticks decoded */
inline Tickval::ticks date_time_parse(const char *ptr, size_t len)
{
	Tickval::ticks result(Tickval::noticks);
   int millisecond(0);
   tm tms = {};

	ptr += parseDate(ptr, 4, tms.tm_year);
	tms.tm_year -= 1900;
	ptr += parseDate(ptr, 2, tms.tm_mon);
	--tms.tm_mon;
	ptr += parseDate(ptr, 2, tms.tm_mday);
	++ptr;
	ptr += parseDate(ptr, 2, tms.tm_hour);
	++ptr;
	ptr += parseDate(ptr, 2, tms.tm_min);
	++ptr;
	ptr += parseDate(ptr, 2, tms.tm_sec);
   switch(len)
   {
	case 21: //_with_ms: // 19981231-23:59:59.123
      parseDate(++ptr, 3, millisecond);
      result = millisecond * Tickval::million; // drop through
   case 17: //: // 19981231-23:59:59
      result += time_to_epoch(tms) * Tickval::billion;
      break;
   default:
      break;
   }

   return result;
}

/*! Decode a DateTime string into ticks
  \param ptr input DateTime string
  \param len length of string
  \return ticks decoded */
inline Tickval::ticks time_parse(const char *ptr, size_t len)
{
	Tickval::ticks result(Tickval::noticks);
   int millisecond(0);
   tm tms = {};

	ptr += parseDate(ptr, 2, tms.tm_hour);
	++ptr;
	ptr += parseDate(ptr, 2, tms.tm_min);
	++ptr;
	ptr += parseDate(ptr, 2, tms.tm_sec);
   switch(len)
   {
	case 12: // 23:59:59.123
      parseDate(++ptr, 3, millisecond);
      result = millisecond * Tickval::million; // drop through
   case 8: // 23:59:59
      result += time_to_epoch(tms) * Tickval::billion;
      break;
   default:
      break;
   }

   return result;
}

inline Tickval::ticks date_parse(const char *ptr, size_t len)
{
   tm tms = {};

	ptr += parseDate(ptr, 4, tms.tm_year);
	tms.tm_year -= 1900;
	ptr += parseDate(ptr, 2, tms.tm_mon);
	--tms.tm_mon;
	if (len == 8)
		parseDate(ptr, 2, tms.tm_mday);
	return time_to_epoch(tms) * Tickval::billion;
}

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimestamp> UTCTimestamp;

/// Partial specialisation for UTCTimestamp field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	Tickval _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field), _value(true) {}

	/*! Copy Ctor.
	  \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const Tickval& val, const RealmBase *rlm=0) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _value(date_time_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field), _value(date_time_parse(from, ::strlen(from))) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=0) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Assignment operator.
	/*! \param that field to assign from
	    \return field */
	Field& operator=(const Field& that)
	{
		if (this != &that)
		{
			_value = that._value;
		}
		return *this;
	}

	/// Dtor.
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
   {
      char buf[MAX_MSGTYPE_FIELD_LEN] = {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _with_ms); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimeOnly> UTCTimeOnly;

/// Partial specialisation for UTCTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	Tickval _value;

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
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _value(time_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field), _value(time_parse(from, ::strlen(from))) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=0) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
   {
      char buf[MAX_MSGTYPE_FIELD_LEN] = {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _time_with_ms); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCDateOnly> UTCDateOnly;

/// Partial specialisation for UTCDateOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<UTCDateOnly, field> : public BaseField
{
	Tickval _value;

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
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _value(date_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field), _value(date_parse(from, ::strlen(from))) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=0) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
   {
      char buf[MAX_MSGTYPE_FIELD_LEN] = {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _date_only); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_LocalMktDate> LocalMktDate;

/// Partial specialisation for LocalMktDate field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<LocalMktDate, field> : public BaseField
{
	Tickval _value;

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
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _value(date_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field), _value(date_parse(from, ::strlen(from))) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=0) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
   {
      char buf[MAX_MSGTYPE_FIELD_LEN] = {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _date_only); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_MonthYear> MonthYear;

/// Partial specialisation for MonthYear field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<MonthYear, field> : public BaseField
{
	size_t _sz;
	Tickval _value;

public:
	/// The FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _sz(from._sz), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _sz(from.size()), _value(date_parse(from.data(), _sz)) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field), _sz(::strlen(from)), _value(date_parse(from, _sz)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=0) : BaseField(field), _sz(6), _value(time_to_epoch(from) * Tickval::billion) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const
   {
      char buf[MAX_MSGTYPE_FIELD_LEN] = {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _sz == 6 ? _short_date_only : _date_only); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimeOnly> TZTimeOnly;

/// Partial specialisation for TZTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<TZTimeOnly, field> : public BaseField
{
	Tickval _value;

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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return 0; }  	// TODO
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimestamp> TZTimestamp;

/// Partial specialisation for TZTimestamp field type.
/*! \tparam field field number (fix tag) */
template<const unsigned short field>
class Field<TZTimestamp, field> : public BaseField
{
	Tickval _value;

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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm) {}

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
	~Field() {}

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& get() const { return _value; }

	/*! Get field value.
	  \return value Tickval& */
	const Tickval& operator()() const { return _value; }

	/*! Set field to the supplied value.
	  \param from value to set */
	void set(const Tickval& from) { _value = from; }

	/*! Copy (clone) this field.
	  \return copy of field */
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os; }  	// TODO

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return 0; }  	// TODO
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
	Field (const unsigned val, const RealmBase *rlm=0) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from.c_str(), rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
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

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(toupper(*from) == 'Y') {}

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
	~Field() {}

	/*! Get field value.
	  \return value (bool) */
	bool get() const { return _value; }

	/*! Get field value.
	  \return value (bool) */
	bool operator()() const { return _value; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value ? 'Y' : 'N') : -1; }

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
	Field *copy() { return new Field(*this); }

	/*! Print this field to the supplied stream. Used to format for FIX output.
	  \param os stream to insert to
	  \return stream */
	std::ostream& print(std::ostream& os) const { return os << (_value ? 'Y' : 'N'); }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { *to = _value ? 'Y' : 'N'; return 1; }
};

//-------------------------------------------------------------------------------------------------
// C++11 will permit proper type aliasing
// typedef EnumType<FieldTrait::ft_float> Qty;
// typedef EnumType<FieldTrait::ft_float> Amt;
// typedef EnumType<FieldTrait::ft_float> price;
// typedef EnumType<FieldTrait::ft_float> PriceOffset;
// typedef EnumType<FieldTrait::ft_float> Percentage;

typedef double Qty;
typedef double Amt;
typedef double price;
typedef double PriceOffset;
typedef double Percentage;

//-------------------------------------------------------------------------------------------------
// C++11 will permit proper type aliasing
// typedef EnumType<FieldTrait::ft_string> MultipleCharValue;
// typedef EnumType<FieldTrait::ft_string> MultipleStringValue;
// typedef EnumType<FieldTrait::ft_string> country;
// typedef EnumType<FieldTrait::ft_string> currency;
// typedef EnumType<FieldTrait::ft_string> Exchange;
// typedef EnumType<FieldTrait::ft_string> Language;
// typedef EnumType<FieldTrait::ft_string> XMLData;

typedef f8String MultipleCharValue;
typedef f8String MultipleStringValue;
typedef f8String country;
typedef f8String currency;
typedef f8String Exchange;
typedef f8String Language;
typedef f8String XMLData;

//-------------------------------------------------------------------------------------------------
/// Field metadata structures
class Inst
{
   template<typename T>
	struct _gen
	{
		static BaseField *_make(const char *from, const RealmBase *db, const int)
			{ return new T(from, db); }
	};

   template<typename T, typename R>
	struct _gen_realm
	{
		static BaseField *_make_realm(const char *from, const RealmBase *db, const int rv)
		{
			return !db || rv < 0 || rv >= db->_sz || db->_dtype != RealmBase::dt_set
				? new T(from, db) : new T(db->get_rlm_val<R>(rv), db);
		}
	};

	static BaseField *dummy(const char *from, const RealmBase *db, const int) { return 0; }

public:
	Inst() : _do(dummy) {}

	BaseField *(&_do)(const char *from, const RealmBase *db, const int);

   template<typename T>
   Inst(Type2Type<T>) : _do(_gen<T>::_make) {}

   template<typename T, typename R>
   Inst(Type2Types<T, R>) : _do(_gen_realm<T, R>::_make_realm) {}
};

template<>
struct Inst::_gen<void *>
{
	static BaseField *_make(const char *from, const RealmBase *db, const int)
		{ return 0; }
};

struct BaseEntry
{
   const Inst _create;
	const RealmBase *_rlm;
	const char *_name, *_comment;
};

//-------------------------------------------------------------------------------------------------
// Common (administrative) msgtypes
const f8String Common_MsgType_HEARTBEAT("0");
const f8String Common_MsgType_TEST_REQUEST("1");
const f8String Common_MsgType_RESEND_REQUEST("2");
const f8String Common_MsgType_REJECT("3");
const f8String Common_MsgType_SEQUENCE_RESET("4");
const f8String Common_MsgType_LOGOUT("5");
const f8String Common_MsgType_LOGON("A");
const char Common_MsgByte_HEARTBEAT('0');
const char Common_MsgByte_TEST_REQUEST('1');
const char Common_MsgByte_RESEND_REQUEST('2');
const char Common_MsgByte_REJECT('3');
const char Common_MsgByte_SEQUENCE_RESET('4');
const char Common_MsgByte_LOGOUT('5');
const char Common_MsgByte_LOGON('A');

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

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_FIELD_HPP_
/* vim: set ts=3 sw=3 tw=0 noet :*/

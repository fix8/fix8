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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_FIELD_HPP_
#define FIX8_FIELD_HPP_

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
template<unsigned field> struct EnumType {};

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

	RealmBase(const void *range, RealmType dtype, FieldTrait::FieldType ftype, int sz, const char * const *descriptions)
		: _range(range), _dtype(dtype), _ftype(ftype), _sz(sz), _descriptions(descriptions) {}

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

	/*! Printer helper
	  \tparam T target type
	  \param os output stream
	  \param idx index of value in range
	  \return the output stream */
	template<typename T>
	std::ostream& _print(std::ostream& os, int idx) const { return os << *((static_cast<const T *>(_range) + idx)); }

	/*! Print the given value by index to the supplied stream
	  \param os output stream
	  \param idx index of value in range
	  \return the output stream */
	std::ostream& print(std::ostream& os, int idx) const
	{
		return FieldTrait::is_int(_ftype) ? _print<int>(os, idx)
			: FieldTrait::is_char(_ftype) ? _print<char>(os, idx)
			: FieldTrait::is_float(_ftype) ? _print<fp_type>(os, idx)
			: FieldTrait::is_string(_ftype) ? _print<f8String>(os, idx) : os;
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
	BaseField(unsigned short fnum, const RealmBase *rlm=nullptr) : _fnum(fnum), _rlm(rlm) {}

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

	/*! Get the underlying FieldType for this field
	  \return field type */
	virtual FieldTrait::FieldType get_underlying_type() const = 0;

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

	/*! Cast this field to the supplied type.
	  \tparam T target type
	  \return pointer to the cast field */
	template<typename T>
	const T *as() const { return static_cast<T*>(this); }

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

	/// BaseField Equivalence test.
	/*! \param that field to compare
	    \return true if same */
	bool same_base(const BaseField& that) const { return that._fnum == _fnum; }

	/// Equivalence operator.
	/*! \param that field to compare
	    \return true if same */
	virtual bool operator==(const BaseField& that) const = 0;

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	virtual bool operator<(const BaseField& that) const = 0;

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	virtual bool operator>(const BaseField& that) const = 0;

	/// Inequivalence operator.
	/*! \param that field to compare
	    \return true if not the same */
	bool operator!=(const BaseField& that) const { return !(*this == that); }

	/// Less or equal to operator.
	/*! \param that field to compare
	    \return true if less than or equal to */
	bool operator<=(const BaseField& that) const { return *this < that || *this == that; }

	/// Greater or equal to operator.
	/*! \param that field to compare
	    \return true if greater than or equal to */
	bool operator>=(const BaseField& that) const { return *this > that || *this == that; }

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
/// Field template. There will ONLY be partial template specialisations of this template.
/*! \tparam T field type
    \tparam field field number (fix tag) */
template<typename T, unsigned short field>
class Field : public BaseField
{
	Field() = delete;
	Field(const Field&) = delete;
	Field& operator=(const Field&) = delete;
	Field(const f8String&, const RealmBase *) = delete;
};

//-------------------------------------------------------------------------------------------------
/// Partial specialisation for int field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<int, field> : public BaseField
{
protected:
	int _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_int;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field), _value() {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const int val, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(fast_atoi<int>(from.c_str())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(fast_atoi<int>(from)) {}

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

	/// Equivalence operator.
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<int, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<int, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<int, field>&>(that)._value; }

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
template<unsigned short field>
class Field<char *, field> : public BaseField
{
protected:
	const char *_value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_data;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(from) {}

	/// Dtor.
	virtual ~Field() {}

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

	/// Equivalence operator.
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && ::strcmp(static_cast<const Field<char *, field>&>(that)._value, _value) == 0; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	virtual bool operator<(const BaseField& that) const
		{ return same_base(that) && ::strcmp(_value, static_cast<const Field<char *, field>&>(that)._value) < 0; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	virtual bool operator>(const BaseField& that) const
		{ return same_base(that) && ::strcmp(_value, static_cast<const Field<char *, field>&>(that)._value) > 0; }

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
template<unsigned short field>
class Field<f8String, field> : public BaseField
{
protected:
	f8String _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_data;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(from) {}

	/// Dtor.
	virtual ~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<f8String, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	virtual bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<f8String, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	virtual bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<f8String, field>&>(that)._value; }

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
/// Partial specialisation for fp_type field type. fp_type is singe or double
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<fp_type, field> : public BaseField
{
protected:
	fp_type _value;
	int _precision;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_float;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field), _value(), _precision(FIX8_DEFAULT_PRECISION) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field, from._rlm), _value(from._value), _precision(from._precision) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const fp_type& val, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(val), _precision(FIX8_DEFAULT_PRECISION) {}

	/*! Value ctor.
	  \param val value to set
	  \param prec precision digits
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const fp_type& val, const int prec, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(val), _precision(prec) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(fast_atof(from.c_str())), _precision(FIX8_DEFAULT_PRECISION) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(fast_atof(from)), _precision(FIX8_DEFAULT_PRECISION) {}

	/// Dtor.
	virtual ~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<fp_type, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<fp_type, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<fp_type, field>&>(that)._value; }

	/*! Set the output precision
	  \param prec precision digits */
	void set_precision(const int prec) { _precision = prec; }

	/*! Check if this value is a member/in range of the domain set.
	  \return true if in the set or no domain available */
	bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }

	/*! Get the realm index of this value in the domain set.
	  \return the index in the domain set of this value */
	int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	/*! Get field value.
	  \return value (fp_type) */
	const fp_type& get() const { return _value; }

	/*! Get field value.
	  \return value (fp_type) */
	const fp_type& operator()() const { return _value; }

	/*! Get field value.
	  \param from value to set
	  \return original value (int) */
	const fp_type& set(const fp_type& from) { return _value = from; }

	/*! Set the value from a string.
	  \param from value to set
	  \return original value (fp_type) */
	const fp_type& set_from_raw(const f8String& from) { return _value = fast_atof(from.c_str()); }

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
template<unsigned short field>
class Field<char, field> : public BaseField
{
	char _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_char;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field), _value() {}

	/*! Copy Ctor.
	  \param from field to copy */
	Field (const Field& from) : BaseField(field, from._rlm), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char& val, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(from[0]) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(*from) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<char, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<char, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<char, field>&>(that)._value; }

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
/*! Format ASCII decimal value
  \param data source value
  \param to target location for string
  \param width len of string
  \return number bytes decoded */
inline void format0(int data, char *to, int width)
{
	while(width-- > 0)
	{
		to[width] = data % 10 + '0';
		data /= 10;
	}
}

/*! Decode ASCII decimal value
  \param begin decode from
  \param len number of bytes in string
  \param to target location for value
  \return number bytes decoded */
inline size_t parse_decimal(const char *begin, size_t len, int &to)
{
	const char *bsv(begin);
	while(len-- > 0)
		to = (to << 3) + (to << 1) + (*begin++ - '0');
	return begin - bsv;
}

/*! Convert tm to time_t
	Based on Ghulam M. Babar's "mktime slow? use custom function"
	see http://gmbabar.wordpress.com/2010/12/01/mktime-slow-use-custom-function/
  \param ltm decode from
  \param utcdiff utc offset in mins
  \return time_t */
inline time_t time_to_epoch (const tm& ltm, int utcdiff=0)
{
   static const int mon_days[] {0,
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
  \param tickval input Tickval object
  \param to output buffer, should make sure there is enough space reserved
  \param ind indicating whether need millisecond or not
	_time_only, the format string will be "HH:MM:SS"
	_time_with_ms, the format string will be "HH:MM:SS.mmm"
	_short_date_only, the format string will be "YYYYMM"
	_date_only, the format string will be "YYYYMMDD"
	_sec_only, the format string will be "YYYYMMDD-HH:MM:SS"
	_with_ms, the format string will be "YYYYMMDD-HH:MM:SS.mmm"
  \return length of formatted string */
inline size_t date_time_format(const Tickval& tickval, char *to, TimeIndicator ind)
{
   const tm result(tickval.get_tm());
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
  \param ptr input DateTime string, if *ptr == '!' return current time
  \param len length of string
  \return ticks decoded */
inline Tickval::ticks date_time_parse(const char *ptr, size_t len)
{
	if (len == 0 || (*ptr == 'n' && len == 3 && *(ptr + 1) == 'o' && *(ptr + 2) == 'w'))	// special cases initialise to 'now'
		return Tickval(true).get_ticks();

	Tickval::ticks result(Tickval::noticks);
	int millisecond(0);
	tm tms {};

	ptr += parse_decimal(ptr, 4, tms.tm_year);
	tms.tm_year -= 1900;
	ptr += parse_decimal(ptr, 2, tms.tm_mon);
	--tms.tm_mon;
	ptr += parse_decimal(ptr, 2, tms.tm_mday);
	++ptr;
	ptr += parse_decimal(ptr, 2, tms.tm_hour);
	++ptr;
	ptr += parse_decimal(ptr, 2, tms.tm_min);
	++ptr;
	ptr += parse_decimal(ptr, 2, tms.tm_sec);
	switch(len)
	{
	case 21: //_with_ms: // 19981231-23:59:59.123
		parse_decimal(++ptr, 3, millisecond);
		result = millisecond * Tickval::million; // drop through
	case 17: //: // 19981231-23:59:59
		result += time_to_epoch(tms) * Tickval::billion;
		break;
	default:
		break;
	}

	return result;
}

/*! Decode a Time string into ticks
  \param ptr input time string, if *ptr == '!' return current time
  \param len length of string
  \param timeonly if true, only calculate ticks for today
  \return ticks decoded */
inline Tickval::ticks time_parse(const char *ptr, size_t len, bool timeonly=false)
{
	if (len == 0 || (*ptr == 'n' && len == 3 && *(ptr + 1) == 'o' && *(ptr + 2) == 'w'))	// special cases initialise to 'now'
		return Tickval(true).get_ticks();

	Tickval::ticks result(Tickval::noticks);
   int millisecond(0);
   tm tms {};

	ptr += parse_decimal(ptr, 2, tms.tm_hour);
	++ptr;
	ptr += parse_decimal(ptr, 2, tms.tm_min);
	++ptr;
	ptr += parse_decimal(ptr, 2, tms.tm_sec);
   switch(len)
   {
	case 12: // 23:59:59.123
      parse_decimal(++ptr, 3, millisecond);
      result = millisecond * Tickval::million; // drop through
   case 8: // 23:59:59
		if (!timeonly)
			result += time_to_epoch(tms) * Tickval::billion;
		else
			result += (tms.tm_hour * 3600ULL + tms.tm_min * 60ULL + tms.tm_sec) * Tickval::billion;
      break;
   default:
      break;
   }

   return result;
}

inline Tickval::ticks date_parse(const char *ptr, size_t len)
{
	if (len == 0 || (*ptr == 'n' && len == 3 && *(ptr + 1) == 'o' && *(ptr + 2) == 'w'))	// special cases initialise to 'now'
		return Tickval(true).get_ticks();

   tm tms {};
	ptr += parse_decimal(ptr, 4, tms.tm_year);
	tms.tm_year -= 1900;
	ptr += parse_decimal(ptr, 2, tms.tm_mon);
	--tms.tm_mon;
	if (len == 8)
		parse_decimal(ptr, 2, tms.tm_mday);
	return time_to_epoch(tms) * Tickval::billion;
}

//-------------------------------------------------------------------------------------------------
using UTCTimestamp = EnumType<FieldTrait::ft_UTCTimestamp>;

/// Partial specialisation for UTCTimestamp field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field), _value(true) {}

	/*! Copy Ctor.
	  \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const Tickval& val, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(val) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_time_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_time_parse(from, from ? ::strlen(from) : 0)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<UTCTimestamp, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<UTCTimestamp, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<UTCTimestamp, field>&>(that)._value; }

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
      char buf[MAX_MSGTYPE_FIELD_LEN] {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _with_ms); }
};

//-------------------------------------------------------------------------------------------------
using UTCTimeOnly = EnumType<FieldTrait::ft_UTCTimeOnly>;

/// Partial specialisation for UTCTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_parse(from, from ? ::strlen(from) : 0)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<UTCTimeOnly, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<UTCTimeOnly, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<UTCTimeOnly, field>&>(that)._value; }

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
      char buf[MAX_MSGTYPE_FIELD_LEN] {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _time_with_ms); }
};

//-------------------------------------------------------------------------------------------------
using UTCDateOnly = EnumType<FieldTrait::ft_UTCDateOnly>;

/// Partial specialisation for UTCDateOnly field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<UTCDateOnly, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_parse(from, from ? ::strlen(from) : 0)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<UTCDateOnly, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<UTCDateOnly, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<UTCDateOnly, field>&>(that)._value; }

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
      char buf[MAX_MSGTYPE_FIELD_LEN] {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _date_only); }
};

//-------------------------------------------------------------------------------------------------
using LocalMktDate = EnumType<FieldTrait::ft_LocalMktDate>;

/// Partial specialisation for LocalMktDate field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<LocalMktDate, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_parse(from.data(), from.size())) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field), _value(date_parse(from, from ? ::strlen(from) : 0)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=nullptr) : BaseField(field), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<LocalMktDate, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<LocalMktDate, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<LocalMktDate, field>&>(that)._value; }

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
      char buf[MAX_MSGTYPE_FIELD_LEN] {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _date_only); }
};

//-------------------------------------------------------------------------------------------------
using MonthYear = EnumType<FieldTrait::ft_MonthYear>;

/// Partial specialisation for MonthYear field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<MonthYear, field> : public BaseField
{
	size_t _sz;
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _sz(from._sz), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field), _sz(from.size()), _value(date_parse(from.data(), _sz)) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field), _sz(from ? ::strlen(from) : 0), _value(date_parse(from, _sz)) {}

	/*! Construct from tm struct
	  \param from string to construct field from
	  \param rlm tm struct with broken out values */
	Field (const tm& from, const RealmBase *rlm=nullptr) : BaseField(field), _sz(6), _value(time_to_epoch(from) * Tickval::billion) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<MonthYear, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<MonthYear, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	virtual bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<MonthYear, field>&>(that)._value; }

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
      char buf[MAX_MSGTYPE_FIELD_LEN] {};
      print(buf);
      return os << buf;
   }

	/*! Print this field to the supplied buffer.
	  \param to buffer to print to
	  \return number bytes encoded */
	size_t print(char *to) const { return date_time_format(_value, to, _sz == 6 ? _short_date_only : _date_only); }
};

//-------------------------------------------------------------------------------------------------
using TZTimeOnly = EnumType<FieldTrait::ft_TZTimeOnly>;

/// Partial specialisation for TZTimeOnly field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<TZTimeOnly, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<TZTimeOnly, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<TZTimeOnly, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<TZTimeOnly, field>&>(that)._value; }

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
using TZTimestamp = EnumType<FieldTrait::ft_TZTimestamp>;

/// Partial specialisation for TZTimestamp field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<TZTimestamp, field> : public BaseField
{
	Tickval _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_string;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm) {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<TZTimestamp, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<TZTimestamp, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<TZTimestamp, field>&>(that)._value; }

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
using Length = EnumType<FieldTrait::ft_Length>;

/// Partial specialisation for Length field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<Length, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (unsigned val, const RealmBase *rlm=nullptr) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : Field<int, field>(from.c_str(), rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
using TagNum = EnumType<FieldTrait::ft_TagNum>;

/// Partial specialisation for TagNum field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<TagNum, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=nullptr) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
using SeqNum = EnumType<FieldTrait::ft_SeqNum>;

/// Partial specialisation for SeqNum field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<SeqNum, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=nullptr) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
using NumInGroup = EnumType<FieldTrait::ft_NumInGroup>;

/// Partial specialisation for NumInGroup field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<NumInGroup, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=nullptr) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
using DayOfMonth = EnumType<FieldTrait::ft_DayOfMonth>;

/// Partial specialisation for DayOfMonth field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<DayOfMonth, field> : public Field<int, field>
{
public:
	/// Ctor. Compiler won't supply this method.
	Field () : Field<int, field>() {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const unsigned& val, const RealmBase *rlm=nullptr) : Field<int, field>(val, rlm) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : Field<int, field>(from) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : Field<int, field>(from, rlm) {}

	/// Dtor.
	~Field() {}
};

//-------------------------------------------------------------------------------------------------
using Boolean = EnumType<FieldTrait::ft_Boolean>;

/// Partial specialisation for Boolean field type.
/*! \tparam field field number (fix tag) */
template<unsigned short field>
class Field<Boolean, field> : public BaseField
{
	bool _value;
	static const FieldTrait::FieldType _ftype = FieldTrait::ft_char;

public:
	/// Get the FIX fieldID (tag number).
	static unsigned short get_field_id() { return field; }

	/// The FieldType
	FieldTrait::FieldType get_underlying_type() const { return _ftype; }

	/// Ctor.
	Field () : BaseField(field) {}

	/*! Value ctor.
	  \param val value to set
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char val, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(toupper(val) == 'Y') {}

	/*! Value ctor.
	  \param val value to set */
	explicit Field (const bool val) : BaseField(field), _value(val) {}

	/// Copy Ctor.
	/* \param from field to copy */
	Field (const Field& from) : BaseField(field), _value(from._value) {}

	/*! Construct from string ctor.
	  \param from string to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const f8String& from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(toupper(from[0]) == 'Y') {}

	/*! Construct from char * ctor.
	  \param from char * to construct field from
	  \param rlm pointer to the realmbase for this field (if available) */
	Field (const char *from, const RealmBase *rlm=nullptr) : BaseField(field, rlm), _value(toupper(*from) == 'Y') {}

	/// Dtor.
	~Field() {}

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
	/*! \param that field to compare
	    \return true if same */
	bool operator==(const BaseField& that) const
		{ return same_base(that) && static_cast<const Field<Boolean, field>&>(that)._value == _value; }

	/// Less than operator.
	/*! \param that field to compare
	    \return true if less than */
	bool operator<(const BaseField& that) const
		{ return same_base(that) && _value < static_cast<const Field<Boolean, field>&>(that)._value; }

	/// Greater than operator.
	/*! \param that field to compare
	    \return true if greater than */
	bool operator>(const BaseField& that) const
		{ return same_base(that) && _value > static_cast<const Field<Boolean, field>&>(that)._value; }

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
using Qty = fp_type;
using Amt = fp_type;
using price = fp_type;
using PriceOffset = fp_type;
using Percentage = fp_type;

//-------------------------------------------------------------------------------------------------
using MultipleCharValue = f8String;
using MultipleStringValue = f8String;
using country = f8String;
using currency = f8String;
using Exchange = f8String;
using Language = f8String;
using XMLData = f8String;
using data = f8String;

//-------------------------------------------------------------------------------------------------
/// Field metadata structures
class Inst
{
	struct _gen
	{
		/*! Instantiate a field (no realm)
			\tparam T type to instantiate
			\param from source string
			\param db realm base for this type
			\param rv realm value
			\return new field */
		template<typename T>
		static BaseField *_make(const char *from, const RealmBase *db, const int rv)
			{ return new T{from, db}; }

		/*! Instantiate a field
			\tparam T type to instantiate
			\param from source string
			\param db realm base for this type
			\param rv realm value
			\return new field */
		template<typename T, typename R>
		static BaseField *_make(const char *from, const RealmBase *db, const int rv)
		{
			return !db || rv < 0 || rv >= db->_sz || db->_dtype != RealmBase::dt_set
				? new T(from, db) : new T(db->get_rlm_val<R>(rv), db);
		}
	};

public:
	BaseField *(&_do)(const char *from, const RealmBase *db, const int);

	template<typename T, typename... args>
	Inst(Type2Type<T, args...>) : _do(_gen::_make<T, args...>) {}
};

struct BaseEntry
{
   const Inst _create;
	const char *_name;
	const unsigned short _fnum;
	const RealmBase *_rlm;
	const char *_comment;
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
const f8String Common_MsgType_BUSINESS_REJECT("j");
const char Common_MsgByte_HEARTBEAT('0');
const char Common_MsgByte_TEST_REQUEST('1');
const char Common_MsgByte_RESEND_REQUEST('2');
const char Common_MsgByte_REJECT('3');
const char Common_MsgByte_SEQUENCE_RESET('4');
const char Common_MsgByte_LOGOUT('5');
const char Common_MsgByte_LOGON('A');
const char Common_MsgByte_BUSINESS_REJECT('j');

//-------------------------------------------------------------------------------------------------
// Common FIX field numbers

const unsigned short Common_BeginSeqNo(7);
const unsigned short Common_BeginString(8);
const unsigned short Common_BodyLength(9);
const unsigned short Common_CheckSum(10);
const unsigned short Common_EndSeqNo(16);
const unsigned short Common_MsgSeqNum(34);
const unsigned short Common_MsgType(35);
const unsigned short Common_NewSeqNo(36);
const unsigned short Common_PossDupFlag(43);
const unsigned short Common_RefSeqNum(45);
const unsigned short Common_SenderCompID(49);
const unsigned short Common_SendingTime(52);
const unsigned short Common_TargetCompID(56);
const unsigned short Common_Text(58);
const unsigned short Common_EncryptMethod(98);
const unsigned short Common_HeartBtInt(108);
const unsigned short Common_TestReqID(112);
const unsigned short Common_OnBehalfOfCompID(115);
const unsigned short Common_OnBehalfOfSubID(116);
const unsigned short Common_OrigSendingTime(122);
const unsigned short Common_GapFillFlag(123);
const unsigned short Common_ResetSeqNumFlag(141);
const unsigned short Common_OnBehalfOfLocationID(144);
const unsigned short Common_OnBehalfOfSendingTime(370);
const unsigned short Common_RefMsgType(372);
const unsigned short Common_BusinessRejectReason(380);
const unsigned short Common_DefaultApplVerID(1137);	// >= 5.0 || FIXT1.1

//-------------------------------------------------------------------------------------------------
// Common FIX fields

using msg_seq_num = Field<SeqNum, Common_MsgSeqNum>;
using begin_seq_num = Field<SeqNum, Common_BeginSeqNo>;
using end_seq_num = Field<SeqNum, Common_EndSeqNo>;
using new_seq_num = Field<SeqNum, Common_NewSeqNo>;
using ref_seq_num = Field<SeqNum, Common_RefSeqNum>;

using body_length = Field<Length, Common_BodyLength>;

using sender_comp_id = Field<f8String, Common_SenderCompID>;
using target_comp_id = Field<f8String, Common_TargetCompID>;
using msg_type = Field<f8String, Common_MsgType>;
using check_sum = Field<f8String, Common_CheckSum>;
using begin_string = Field<f8String, Common_BeginString>;
using test_request_id = Field<f8String, Common_TestReqID>;
using text = Field<f8String, Common_Text>;
using default_appl_ver_id = Field<f8String, Common_DefaultApplVerID>;
using ref_msg_type = Field<f8String, Common_RefMsgType>;

using sending_time = Field<UTCTimestamp, Common_SendingTime>;
using orig_sending_time = Field<UTCTimestamp, Common_OrigSendingTime>;

using gap_fill_flag = Field<Boolean, Common_GapFillFlag>;
using poss_dup_flag = Field<Boolean, Common_PossDupFlag>;
using reset_seqnum_flag = Field<Boolean, Common_ResetSeqNumFlag>;

using heartbeat_interval = Field<int, Common_HeartBtInt>;
using encrypt_method = Field<int, Common_EncryptMethod>;
using business_reject_reason = Field<int, Common_BusinessRejectReason>;

using onbehalfof_comp_id = Field<f8String, Common_OnBehalfOfCompID>;
using onbehalfof_sub_id = Field<f8String, Common_OnBehalfOfSubID>;
using onbehalfof_location_id = Field<f8String, Common_OnBehalfOfLocationID>;
using onbehalfof_sending_time = Field<UTCTimestamp, Common_OnBehalfOfSendingTime>;

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_FIELD_HPP_

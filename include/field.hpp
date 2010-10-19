//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2007-2010, David L. Dight <fix@fix8.org>
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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

$Id$
$LastChangedDate$
$Rev$
$URL$

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_FIELD_HPP_
#define _FIX8_FIELD_HPP_

#include <Poco/DateTime.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
// magic FIX field numbers
const unsigned Magic_BeginString(8);
const unsigned Magic_BodyLength(9);
const unsigned Magic_CheckSum(10);
const unsigned Magic_MsgType(35);

//-------------------------------------------------------------------------------------------------
template<unsigned field>
struct EnumType
{
	enum { val = field };
};

//-------------------------------------------------------------------------------------------------
// domain range
struct DomainBase
{
	enum DomType { dt_range, dt_set };

	const void *_range;
	DomType _dtype;
	FieldTrait::FieldType _ftype;
	const size_t _sz;
	const char **_descriptions;

	template<typename T>
	const bool is_valid(const T& what) const
	{
		return _dtype == dt_set ? std::binary_search(static_cast<const T*>(_range), static_cast<const T*>(_range) + _sz, what)
									   : *static_cast<const T*>(_range) <= what && what <= *(static_cast<const T*>(_range) + 1);
	}
};

//-------------------------------------------------------------------------------------------------
class BaseField
{
	const unsigned short _fnum;

protected:
	const DomainBase *_dom;

public:
	BaseField(const unsigned short fnum, const DomainBase *dom=0) : _fnum(fnum), _dom(dom) {}
	virtual ~BaseField() {}

	const unsigned short get_tag() const { return _fnum; }
	virtual std::string get_raw() const
	{
		std::ostringstream ostr;
		print(ostr);
		return ostr.str();
	}

	virtual std::ostream& print(std::ostream& os) const = 0;
	virtual bool is_valid() const { return true; }

	template<typename T>
	const T& from() const { return *static_cast<const T*>(this); }

	size_t encode(std::ostream& os)
	{
		const std::ios::pos_type where(os.tellp());
		os << _fnum << '=' << *this << '\x01';
		return os.tellp() - where;
	}

	friend std::ostream& operator<<(std::ostream& os, const BaseField& what) { return what.print(os); }
	friend class MessageBase;
};

template<typename T, const unsigned short field>
class Field : public BaseField
{
public:
	Field () : BaseField(field) {}
	Field(const std::string& from, const DomainBase *dom=0) : BaseField(field, dom) {}
	virtual ~Field() {}

	virtual const T& get() = 0;
	virtual const T& set(const T& from) = 0;
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<int, field> : public BaseField
{
protected:
	int _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const int val) : BaseField(field), _value(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field, dom), _value(GetValue<int>(from)) {}
	virtual ~Field() {}
	virtual bool is_valid() const { return _dom ? _dom->is_valid(_value) : true; }

	const int& get() { return _value; }
	const int& set(const int& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<std::string, field> : public BaseField
{
protected:
	std::string _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field, dom), _value(from) {}
	virtual ~Field() {}
	virtual bool is_valid() const { return _dom ? _dom->is_valid(_value) : true; }

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<double, field> : public BaseField
{
protected:
	double _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const double& val) : BaseField(field), _value(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field, dom), _value(GetValue<double>(from)) {}
	virtual ~Field() {}
	virtual bool is_valid() const { return _dom ? _dom->is_valid(_value) : true; }

	const double& get() { return _value; }
	const double& set(const double& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<char, field> : public BaseField
{
	char _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const char& val) : BaseField(field), _value(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field, dom), _value(from[0]) {}
	virtual ~Field() {}
	virtual bool is_valid() const { return _dom ? _dom->is_valid(_value) : true; }

	const char& get() { return _value; }
	const char& set(const char& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_MonthYear> MonthYear;

template<const unsigned short field>
class Field<MonthYear, field> : public Field<std::string, field>
{
public:
	Field () : Field<std::string, field>(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<std::string, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_data> data;

template<const unsigned short field>
class Field<data, field> : public Field<std::string, field>
{
public:
	Field () : Field<std::string, field>(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<std::string, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimestamp> UTCTimestamp;

template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0)  : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimeOnly> UTCTimeOnly;

template<const unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCDateOnly> UTCDateOnly;

template<const unsigned short field>
class Field<UTCDateOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_LocalMktDate> LocalMktDate;

template<const unsigned short field>
class Field<LocalMktDate, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimeOnly> TZTimeOnly;

template<const unsigned short field>
class Field<TZTimeOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TZTimestamp> TZTimestamp;

template<const unsigned short field>
class Field<TZTimestamp, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field) {}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
	virtual bool is_valid() const { return _dom ? _dom->is_valid(_value) : true; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Length> Length;

template<const unsigned short field>
class Field<Length, field> : public Field<int, field>
{
public:
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<int, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TagNum> TagNum;

template<const unsigned short field>
class Field<TagNum, field> : public Field<int, field>
{
public:
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<int, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_SeqNum> SeqNum;

template<const unsigned short field>
class Field<SeqNum, field> : public Field<int, field>
{
public:
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<int, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_NumInGroup> NumInGroup;

template<const unsigned short field>
class Field<NumInGroup, field> : public Field<int, field>
{
public:
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<int, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_DayOfMonth> DayOfMonth;

template<const unsigned short field>
class Field<DayOfMonth, field> : public Field<int, field>
{
public:
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<int, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Boolean> Boolean;

template<const unsigned short field>
class Field<Boolean, field> : public BaseField
{
	bool _value;

public:
	Field () : BaseField(field) {}
	Field (const bool& val) : BaseField(field), _value(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : BaseField(field), _value(toupper(from[0]) == 'Y') {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << (_value ? 'Y' : 'N'); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_float> Qty;
typedef EnumType<FieldTrait::ft_float> Amt;
typedef EnumType<FieldTrait::ft_float> price;
typedef EnumType<FieldTrait::ft_float> PriceOffset;
typedef EnumType<FieldTrait::ft_float> Percentage;

template<const unsigned short field>
class Field<Qty, field> : public Field<double, field>
{
public:
	Field (const double& val) : Field<double, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<double, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_string> MultipleCharValue;
typedef EnumType<FieldTrait::ft_string> MultipleStringValue;
typedef EnumType<FieldTrait::ft_string> country;
typedef EnumType<FieldTrait::ft_string> currency;
typedef EnumType<FieldTrait::ft_string> Exchange;

template<const unsigned short field>
class Field<MultipleCharValue, field> : public Field<std::string, field>
{
public:
	Field (const std::string& val) : Field<std::string, field>(val) {}
	Field (const std::string& from, const DomainBase *dom=0) : Field<std::string, field>(from, dom) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
struct BaseEntry
{
	BaseField *(*_create)(const std::string&, const DomainBase*);
	const DomainBase *_dom;
	const char *_name, *_comment;
};

} // FIX8

#endif // _FIX8_FIELD_HPP_

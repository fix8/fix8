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

#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
template<unsigned field>
struct EnumType
{
	enum { val = field };
};

//-------------------------------------------------------------------------------------------------
struct RealmBase	// metadata domain
{
	enum RealmType { dt_range, dt_set };

	const void *_range;
	RealmType _dtype;
	FieldTrait::FieldType _ftype;
	const size_t _sz;
	const char **_descriptions;

	template<typename T>
	const bool is_valid(const T& what) const
	{
		const T *rng(static_cast<const T*>(_range));
		return _dtype == dt_set ? std::binary_search(rng, rng + _sz, what) : *rng <= what && what <= *(rng + 1);
	}

	template<typename T>
	const int get_rlm_idx(const T& what) const
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
class BaseField : public f8Base
{
	unsigned short _fnum;

protected:
	const RealmBase *_rlm;

public:
	BaseField(const unsigned short fnum, const RealmBase *rlm=0) : _fnum(fnum), _rlm(rlm) {}
	virtual ~BaseField() {}

	const unsigned short get_tag() const { return _fnum; }
	virtual std::string get_raw() const
	{
		std::ostringstream ostr;
		print(ostr);
		return ostr.str();
	}

	virtual std::ostream& print(std::ostream& os) const = 0;
	virtual BaseField *copy() = 0;
	virtual bool is_valid() const { return true; }
	virtual int get_rlm_idx() const { return -1; }

	template<typename T>
	T& from() { return *static_cast<const T*>(this); }

	size_t encode(std::ostream& os)
	{
		const std::ios::pos_type where(os.tellp());
		os << _fnum << '=' << *this << default_field_separator;
		return os.tellp() - where;
	}

	friend std::ostream& operator<<(std::ostream& os, const BaseField& what) { return what.print(os); }
	friend class MessageBase;
};

//-------------------------------------------------------------------------------------------------
template<typename T, const unsigned short field>
class Field : public BaseField
{
public:
	Field () : BaseField(field) {}
	Field(const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm) {}
	virtual ~Field() {}

	virtual const T& get() = 0;
	virtual const T& operator()() = 0;
	virtual const T& set(const T& from) = 0;
	virtual const T& set_from_raw(const f8String& from) = 0;
	virtual Field *copy() = 0;
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<int, field> : public BaseField
{
protected:
	int _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const int val) : BaseField(field), _value(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(GetValue<int>(from)) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	const int& get() { return _value; }
	const int& operator()() { return _value; }
	const int& set(const int& from) { return _value = from; }
	const int& set_from_raw(const f8String& from) { return _value = GetValue<int>(from); }
	virtual Field *copy() { return new Field(*this); }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<f8String, field> : public BaseField
{
protected:
	f8String _value;

public:
	Field () : BaseField(field) {}
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	const f8String& get() { return _value; }
	const f8String& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	const f8String& set_from_raw(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const double& val) : BaseField(field), _value(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(GetValue<double>(from)) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	virtual const double& get() { return _value; }
	virtual const double& operator()() { return _value; }
	virtual const double& set(const double& from) { return _value = from; }
	virtual const double& set_from_raw(const f8String& from) { return _value = GetValue<double>(from); }
	virtual Field *copy() { return new Field(*this); }
	virtual std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<char, field> : public BaseField
{
	char _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const char& val) : BaseField(field), _value(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field, rlm), _value(from[0]) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}
	virtual bool is_valid() const { return _rlm ? _rlm->is_valid(_value) : true; }
	virtual int get_rlm_idx() const { return _rlm ? _rlm->get_rlm_idx(_value) : -1; }

	const char& get() { return _value; }
	const char& operator()() { return _value; }
	const char& set(const char& from) { return _value = from; }
	const char& set_from_raw(const f8String& from) { return _value = from[0]; }
	virtual Field *copy() { return new Field(*this); }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_MonthYear> MonthYear;

template<const unsigned short field>
class Field<MonthYear, field> : public Field<f8String, field>
{
public:
	Field () : Field<f8String, field>(field) {}
	Field (const Field& from) : Field<f8String, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_data> data;

template<const unsigned short field>
class Field<data, field> : public Field<f8String, field>
{
public:
	Field () : Field<f8String, field>(field) {}
	Field (const Field& from) : Field<f8String, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimestamp> UTCTimestamp;

template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	static const std::string _fmt_sec, _fmt_ms;
	static const size_t _sec_only = 17, _with_ms = 21;
	Poco::DateTime _value;
	int _tzdiff;

public:
	Field () : BaseField(field), _tzdiff() {}
	Field (const Field& from) : BaseField(field), _value(from._value), _tzdiff(from._tzdiff) {}
	Field (const Poco::DateTime& val) : BaseField(field), _value(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field)
	{
		if (from.size() == _sec_only) // 19981231-23:59:59
			Poco::DateTimeParser::parse(_fmt_sec, from, _value, _tzdiff);
		else if (from.size() == _with_ms) // 19981231-23:59:59.123
			Poco::DateTimeParser::parse(_fmt_ms, from, _value, _tzdiff);
	}
	Field& operator=(const Field& that)
	{
		if (this != &that)
		{
			_value = that._value;
			_tzdiff = that._tzdiff;
		}
		return *this;
	}

	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	void set(const Poco::DateTime& from) { _value = from; }
	virtual Field *copy() { return new Field(*this); }
	std::ostream& print(std::ostream& os) const
		{ return os << Poco::DateTimeFormatter::format(_value, _fmt_sec); }

};

template<const unsigned short field>
const std::string Field<UTCTimestamp, field>::_fmt_sec("%Y%m%d-%H:%M:%S");
template<const unsigned short field>
const std::string Field<UTCTimestamp, field>::_fmt_ms("%Y%m%d-%H:%M:%S.%i");

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimeOnly> UTCTimeOnly;

template<const unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field) {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const Poco::DateTime& operator()() { return _value; }
	const f8String& set(const f8String& from) { return _value = from; }
	virtual Field *copy() { return new Field(*this); }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Length> Length;

template<const unsigned short field>
class Field<Length, field> : public Field<int, field>
{
public:
	Field () : Field<int, field>() {}
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const Field& from) : Field<int, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TagNum> TagNum;

template<const unsigned short field>
class Field<TagNum, field> : public Field<int, field>
{
public:
	Field () : Field<int, field>() {}
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const Field& from) : Field<int, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_SeqNum> SeqNum;

template<const unsigned short field>
class Field<SeqNum, field> : public Field<int, field>
{
public:
	Field () : Field<int, field>() {}
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const Field& from) : Field<int, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_NumInGroup> NumInGroup;

template<const unsigned short field>
class Field<NumInGroup, field> : public Field<int, field>
{
public:
	Field () : Field<int, field>() {}
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const Field& from) : Field<int, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_DayOfMonth> DayOfMonth;

template<const unsigned short field>
class Field<DayOfMonth, field> : public Field<int, field>
{
public:
	Field () : Field<int, field>() {}
	Field (const unsigned& val) : Field<int, field>(val) {}
	Field (const Field& from) : Field<int, field>(from) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<int, field>(from, rlm) {}
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
	Field (const Field& from) : BaseField(field), _value(from._value) {}
	Field (const f8String& from, const RealmBase *rlm=0) : BaseField(field), _value(toupper(from[0]) == 'Y') {}
	Field& operator=(const Field& that)
	{
		if (this != &that)
			_value = that._value;
		return *this;
	}
	virtual ~Field() {}

	const bool get() { return _value; }
	const bool operator()() { return _value; }
	const bool set(const bool from) { return _value = from; }
	const bool set_from_raw(const f8String& from) { return toupper(from[0]) == 'Y'; }
	virtual Field *copy() { return new Field(*this); }
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
	Field () : Field<double, field>() {}
	Field (const double& val) : Field<double, field>(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<double, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_string> MultipleCharValue;
typedef EnumType<FieldTrait::ft_string> MultipleStringValue;
typedef EnumType<FieldTrait::ft_string> country;
typedef EnumType<FieldTrait::ft_string> currency;
typedef EnumType<FieldTrait::ft_string> Exchange;

template<const unsigned short field>
class Field<MultipleCharValue, field> : public Field<f8String, field>
{
public:
	Field (const f8String& val) : Field<f8String, field>(val) {}
	Field (const f8String& from, const RealmBase *rlm=0) : Field<f8String, field>(from, rlm) {}
	virtual ~Field() {}
};

//-------------------------------------------------------------------------------------------------
struct BaseEntry
{
	BaseField *(*_create)(const f8String&, const RealmBase*);
	const RealmBase *_rlm;
	const char *_name, *_comment;
};

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
const unsigned Common_DefaultApplVerID(1137);	// >= 5.0 || FIXT1.1

// Common msgtypes
const f8String Common_MsgType_HEARTBEAT("0");
const f8String Common_MsgType_TEST_REQUEST("1");
const f8String Common_MsgType_RESEND_REQUEST("2");
const f8String Common_MsgType_REJECT("3");
const f8String Common_MsgType_SEQUENCE_RESET("4");
const f8String Common_MsgType_LOGOUT("5");
const f8String Common_MsgType_LOGON("A");

//-------------------------------------------------------------------------------------------------
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
typedef Field<f8String, Common_Text> text;;

typedef Field<UTCTimestamp, Common_SendingTime> sending_time;
typedef Field<UTCTimestamp, Common_OrigSendingTime> orig_sending_time;

typedef Field<Boolean, Common_GapFillFlag> gap_fill_flag;
typedef Field<Boolean, Common_PossDupFlag> poss_dup_flag;

typedef Field<int, Common_HeartBtInt> heartbeat_interval;
typedef Field<int, Common_EncryptMethod> encrypt_method;

} // FIX8

#endif // _FIX8_FIELD_HPP_

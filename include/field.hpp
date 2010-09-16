//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_FIELD_HPP_
#define _IF_FIX8_FIELD_HPP_

#include <Poco/DateTime.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
template<unsigned field>
struct EnumType
{
	enum { val = field };
};

//-------------------------------------------------------------------------------------------------
// domain range
class DomainBase
{
public:
	enum DomType { dt_range, dt_set };

protected:
	static const RegExp _domainSpec;

	const void *_range;
	DomType _dtype;
	FieldTrait::FieldType _ftype;
	const size_t _sz;

public:
	DomainBase(const void *range, const DomType dt, const FieldTrait::FieldType ftype, const size_t sz)
		: _range(range), _dtype(dt), _ftype(ftype), _sz(sz) {}
};

template<typename T>
class Domain : public DomainBase
{
public:
	Domain(const DomType dt, const FieldTrait::FieldType ftype, const T *range, const size_t sz)
		: DomainBase(static_cast<void*>(range), dt, ftype, sz) {}

	const bool isValid(const T& what)
	{
		return _dtype == dt_set ? std::binary_search(static_cast<T*>(_range), static_cast<T*>(_range) + _sz, what)
									   : *static_cast<T*>(_range) <= what && what <= *(static_cast<T*>(_range) + 1);
	}
};

//-------------------------------------------------------------------------------------------------
class BaseField
{
	const unsigned short _fnum;
	DomainBase *_dom;

public:
	BaseField(const unsigned short fnum, DomainBase *dom=0) : _fnum(fnum), _dom(dom) {}
	virtual ~BaseField() { delete _dom; }

	const unsigned short get_tag() const { return _fnum; }
	virtual std::string get_raw() const
	{
		std::ostringstream ostr;
		print(ostr);
		return ostr.str();
	}

	virtual std::ostream& print(std::ostream& os) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const BaseField& what) { return what.print(os); }
};

template<typename T, const unsigned short field>
class Field : public BaseField
{
public:
	Field () : BaseField(field) {}
	Field(const std::string& from) {}
	virtual ~Field() {}

	virtual const T& get() = 0;
	virtual const T& set(const T& from) = 0;
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<int, field> : public BaseField
{
	int _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const int val) : BaseField(field), _value(val) {}
	Field (const std::string& from, DomainBase *dom=0);
	virtual ~Field() {}

	const int& get() { return _value; }
	const int& set(const int& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<std::string, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from, DomainBase *dom=0);
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<double, field> : public BaseField
{
	double _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const double& val) : BaseField(field), _value(val) {}
	Field (const std::string& from, DomainBase *dom=0);
	virtual ~Field() {}

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
	Field (const std::string& from, DomainBase *dom=0);
	virtual ~Field() {}

	const char& get() { return _value; }
	const char& set(const char& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
template<const unsigned short field>
class Field<bool, field> : public BaseField
{
	bool _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const bool& val) : BaseField(field), _value(val) {}
	Field (const std::string& from) : BaseField(field), _value(from[0] == 'Y') {}
	virtual ~Field() {}

	const bool& get() { return _value; }
	const bool& set(const bool& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << (_value ? 'Y' : 'N'); }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_MonthYear> MonthYear;

template<const unsigned short field>
class Field<MonthYear, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from) : BaseField(field), _value(from) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_UTCTimestamp> UTCTimestamp;

template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	Poco::DateTime _value;

public:
	Field () : BaseField(field) {}
	Field (const std::string& from);
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
	Field (const std::string& from) : BaseField(field) {}
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
	Field (const std::string& from) : BaseField(field) {}
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
	Field (const std::string& from) : BaseField(field) {}
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
	Field (const std::string& from) : BaseField(field) {}
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
	Field (const std::string& from);
	virtual ~Field() {}

	const Poco::DateTime& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_Length> Length;

template<const unsigned short field>
class Field<Length, field> : public BaseField
{
	unsigned _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const unsigned& val) : BaseField(field), _value(val) {}
	Field (const std::string& from);
	virtual ~Field() {}

	const unsigned get() { return _value; }
	const std::string& set(const unsigned& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_TagNum> TagNum;

template<const unsigned short field>
class Field<TagNum, field> : public BaseField
{
	unsigned _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const unsigned& val) : BaseField(field), _value(val) {}
	Field (const std::string& from);
	virtual ~Field() {}

	const unsigned get() { return _value; }
	const std::string& set(const unsigned& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_SeqNum> SeqNum;

template<const unsigned short field>
class Field<SeqNum, field> : public BaseField
{
	unsigned _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const unsigned& val) : BaseField(field), _value(val) {}
	Field (const std::string& from);
	virtual ~Field() {}

	const unsigned get() { return _value; }
	const std::string& set(const unsigned& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_NumInGroup> NumInGroup;

template<const unsigned short field>
class Field<NumInGroup, field> : public BaseField
{
	unsigned _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const unsigned& val) : BaseField(field), _value(val) {}
	Field (const std::string& from);
	virtual ~Field() {}

	const unsigned get() { return _value; }
	const std::string& set(const unsigned& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

//-------------------------------------------------------------------------------------------------
typedef EnumType<FieldTrait::ft_DayOfMonth> DayOfMonth;

template<const unsigned short field>
class Field<DayOfMonth, field> : public BaseField
{
	unsigned _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const unsigned& val) : BaseField(field), _value(val) {}
	Field (const std::string& from);
	virtual ~Field() {}

	const unsigned get() { return _value; }
	const std::string& set(const unsigned& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

} // FIX8

#endif // _IF_FIX8_FIELD_HPP_

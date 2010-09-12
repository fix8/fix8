//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_FIELD_HPP_
#define _IF_FIX8_FIELD_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

class BaseField
{
	const unsigned short _fnum;

public:
	BaseField(const unsigned short fnum) : _fnum(fnum) {}
	virtual ~BaseField() {}

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
	virtual ~Field() {}

	virtual const T& get() = 0;
	virtual const T& set(const T& from) = 0;
};

template<const unsigned short field>
class Field<int, field> : public BaseField
{
	int _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const int val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const int& get() { return _value; }
	const int& set(const int& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

template<const unsigned short field>
class Field<std::string, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

template<const unsigned short field>
class Field<double, field> : public BaseField
{
	double _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const double& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const double& get() { return _value; }
	const double& set(const double& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

template<const unsigned short field>
class Field<char, field> : public BaseField
{
	char _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const double& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const char& get() { return _value; }
	const char& set(const char& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

template<const unsigned short field>
class Field<bool, field> : public BaseField
{
	bool _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const bool& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const bool& get() { return _value; }
	const bool& set(const bool& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

class UTCTimestamp
{
};

template<const unsigned short field>
class Field<UTCTimestamp, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};


class UTCTimeOnly
{
};

template<const unsigned short field>
class Field<UTCTimeOnly, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

class UTCDateOnly
{
};

template<const unsigned short field>
class Field<UTCDateOnly, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

class LocalMktDate
{
};

template<const unsigned short field>
class Field<LocalMktDate, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

class TZTimeOnly
{
};

template<const unsigned short field>
class Field<TZTimeOnly, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

class TZTimestamp
{
};

template<const unsigned short field>
class Field<TZTimestamp, field> : public BaseField
{
	std::string _value;

public:
	Field () : BaseField(field), _value() {}
	Field (const std::string& val) : BaseField(field), _value(val) {}
	virtual ~Field() {}

	const std::string& get() { return _value; }
	const std::string& set(const std::string& from) { return _value = from; }
	std::ostream& print(std::ostream& os) const { return os << _value; }
};

} // FIX8

#endif // _IF_FIX8_FIELD_HPP_

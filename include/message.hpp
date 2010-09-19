//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_MESSAGE_HPP_
#define _IF_FIX8_MESSAGE_HPP_

#include <map>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

// all fields present in a message - mandatory and optional
typedef std::map<unsigned short, BaseField *> Fields;

class MessageBase
{
public:
	template<typename InputIterator>
	MessageBase(const InputIterator begin, const InputIterator end) : _fp(begin, end) {}

	MessageBase() {}

	Fields _fields;
	FieldTraits _fp;
};

class Message : public MessageBase
{
protected:
	MessageBase _header, _trailer;

public:
	template<typename InputIterator>
	Message(const InputIterator begin, const InputIterator end) : MessageBase(begin, end) {}

	Message() {}
};

} // FIX8

#endif // _IF_FIX8_MESSAGE_HPP_

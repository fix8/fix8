//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_MESSAGE_HPP_
#define _IF_FIX8_MESSAGE_HPP_

#include <map>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

// all fields present in a message - mandatory and optional
typedef std::map<unsigned short, BaseField *> Fields;
typedef std::multimap<unsigned short, BaseField *> Positions;

class MessageBase
{
public:
	template<typename InputIterator>
	MessageBase(const InputIterator begin, const InputIterator end) : _fp(begin, end) {}

	MessageBase() {}

	Fields _fields;
	FieldTraits _fp;
	Positions _pos;
};

class header;
class trailer;

class Message : public MessageBase
{
protected:
	header *_header;
	trailer *_trailer;

public:
	template<typename InputIterator>
	Message(const InputIterator begin, const InputIterator end)
		: MessageBase(begin, end), _header(), _trailer() {}

	Message() {}
};

//-------------------------------------------------------------------------------------------------
struct BaseMsgEntry
{
	Message *(*_create)(const std::string&, const BaseMsgEntry*);
	const char *_comment;
};

} // FIX8

#endif // _IF_FIX8_MESSAGE_HPP_

//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_MESSAGE_HPP_
#define _IF_FIX8_MESSAGE_HPP_

#include <map>
#include <vector>

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
	virtual ~MessageBase()
		{ std::for_each (_fields.begin(), _fields.end(), free_ptr<Delete2ndPairObject<> >()); }

	unsigned decode(const std::string& from, unsigned offset=0);
	unsigned setupPositions();

	Fields _fields;
	FieldTraits _fp;
	Positions _pos;
};

//-------------------------------------------------------------------------------------------------
class GroupBase
{
	std::vector<MessageBase *> _msgs;

public:
	GroupBase() {}
	virtual ~GroupBase() { std::for_each (_msgs.begin(), _msgs.end(), free_ptr<>()); }

	virtual MessageBase *Create_Group() = 0;
	void add(MessageBase *what) { _msgs.push_back(what); }
	MessageBase *operator[](unsigned where) { return where < _msgs.size() ? _msgs[where] : 0; }
	MessageBase *at(unsigned where) { return operator[](where); }
};

typedef std::map<unsigned short, GroupBase *> Groups;

class Message : public MessageBase
{
protected:
	MessageBase *_header, *_trailer;
	Groups _groups;

public:
	template<typename InputIterator>
	Message(const InputIterator begin, const InputIterator end)
		: MessageBase(begin, end), _header(), _trailer() {}

	Message() {}
	virtual ~Message()
	{
		delete _header;
		delete _trailer;
		std::for_each (_groups.begin(), _groups.end(), free_ptr<Delete2ndPairObject<> >());
	}
};

//-------------------------------------------------------------------------------------------------
struct BaseMsgEntry
{
	Message *(*_create)(const std::string&);
	const char *_comment;
};

} // FIX8

#endif // _IF_FIX8_MESSAGE_HPP_

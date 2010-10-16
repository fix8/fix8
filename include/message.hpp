//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_MESSAGE_HPP_
#define _IF_FIX8_MESSAGE_HPP_

#include <map>
#include <vector>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class MessageBase;

class F8MetaCntx;

class GroupBase
{
	std::vector<MessageBase *> _msgs;

public:
	GroupBase() {}
	virtual ~GroupBase() { clear(); }

	virtual MessageBase *create_group() = 0;
	void add(MessageBase *what) { _msgs.push_back(what); }
	size_t size() const { return _msgs.size(); }
	MessageBase *operator[](unsigned where) { return where < _msgs.size() ? _msgs[where] : 0; }

	void clear()
	{
		std::for_each (_msgs.begin(), _msgs.end(), free_ptr<>());
		_msgs.clear();
	}

	friend class MessageBase;
};

typedef std::map<unsigned short, GroupBase *> Groups;

//-------------------------------------------------------------------------------------------------
typedef std::map<unsigned short, BaseField *> Fields;
typedef std::multimap<unsigned short, BaseField *> Positions;

class MessageBase
{
protected:
	static RegExp _elmnt;

	Fields _fields;
	FieldTraits _fp;
	Positions _pos;
	Groups _groups;
	const f8String& _msgType;
	const class F8MetaCntx& _ctx;

	GroupBase *find_group(const unsigned short fnum)
	{
		Groups::const_iterator gitr(_groups.find(fnum));
		return gitr != _groups.end() ? gitr->second : 0;
	}

	void add_field(const unsigned short fnum, const unsigned pos, BaseField *what)
	{
		_fields.insert(Fields::value_type(fnum, what));
		_pos.insert(Positions::value_type(pos, what));
	}

public:
	template<typename InputIterator>
	MessageBase(const class F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: _fp(begin, end), _msgType(msgType), _ctx(ctx) {}

	//MessageBase() {}
	virtual ~MessageBase() { clear(); }

	void clear()
	{
		std::for_each (_fields.begin(), _fields.end(), free_ptr<Delete2ndPairObject<> >());
		_fields.clear();
		std::for_each (_groups.begin(), _groups.end(), free_ptr<Delete2ndPairObject<> >());
		_groups.clear();
		_fp.clearFlag(FieldTrait::present);
		_pos.clear();
	}

	unsigned decode(const f8String& from, const unsigned offset);
	unsigned decode_group(const unsigned short fnum, const f8String& from, const unsigned offset);
	unsigned encode(f8String& to, const unsigned offset);
	unsigned encode_group(const unsigned short fnum, f8String& to, const unsigned offset);
	unsigned check_positions();

	virtual void print(std::ostream& os);
};

//-------------------------------------------------------------------------------------------------
class Message : public MessageBase
{
protected:
	MessageBase *_header, *_trailer;

public:
	template<typename InputIterator>
	Message(const F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: MessageBase(ctx, msgType, begin, end), _header(), _trailer() {}

	//Message() {}
	virtual ~Message()
	{
		delete _header;
		delete _trailer;
	}

	unsigned decode(const f8String& from);
	unsigned encode(f8String& to);

	virtual void print(std::ostream& os);
};

//-------------------------------------------------------------------------------------------------
struct BaseMsgEntry
{
	Message *(*_create)();
	const char *_name, *_comment;
};

//-------------------------------------------------------------------------------------------------
// metadata context object
struct F8MetaCntx
{
	const unsigned _version;
	const GeneratedTable<const f8String, BaseMsgEntry>& _bme;
	const GeneratedTable<unsigned, BaseEntry>& _be;
	Message *(*_create_header)(), *(*_create_trailer)();
	const f8String _beginStr;

	F8MetaCntx(const unsigned version, const GeneratedTable<const f8String, BaseMsgEntry>& bme,
		const GeneratedTable<unsigned, BaseEntry>& be, const f8String& bg) :
			 _version(version), _bme(bme), _be(be),
			_create_header(_bme.find_ptr("header")->_create),
			_create_trailer(_bme.find_ptr("trailer")->_create), _beginStr(bg) {}

	const unsigned version() const { return _version; }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _IF_FIX8_MESSAGE_HPP_

//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_MESSAGE_HPP_
#define _FIX8_MESSAGE_HPP_

#include <map>
#include <vector>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class MessageBase;
class Message;
typedef std::vector<MessageBase *> GroupElement;

class F8MetaCntx;

//-------------------------------------------------------------------------------------------------
class GroupBase
{
	unsigned short _fnum;
	GroupElement _msgs;

public:
	GroupBase(const unsigned short fnum) : _fnum(fnum) {}
	virtual ~GroupBase() { clear(); }

	virtual MessageBase *create_group() = 0;
	void add(MessageBase *what) { _msgs.push_back(what); }
	void operator+=(MessageBase *what) { add(what); }
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
	Message *(*_mk_hdr)(), *(*_mk_trl)();
	const f8String _beginStr;

	F8MetaCntx(const unsigned version, const GeneratedTable<const f8String, BaseMsgEntry>& bme,
		const GeneratedTable<unsigned, BaseEntry>& be, const f8String& bg) :
			 _version(version), _bme(bme), _be(be),
			_mk_hdr(_bme.find_ptr("header")->_create),
			_mk_trl(_bme.find_ptr("trailer")->_create), _beginStr(bg) {}

	const unsigned version() const { return _version; }
};

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

	void add_field(const unsigned short fnum, const unsigned pos, BaseField *what)
	{
		_fields.insert(Fields::value_type(fnum, what));
		_pos.insert(Positions::value_type(pos, what));
		_fp.set(fnum, FieldTrait::present);
	}

public:
	template<typename InputIterator>
	MessageBase(const class F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: _fp(begin, end), _msgType(msgType), _ctx(ctx) {}

	virtual ~MessageBase() { clear(); }

	void clear()
	{
		std::for_each (_fields.begin(), _fields.end(), free_ptr<Delete2ndPairObject<> >());
		_fields.clear();
		std::for_each (_groups.begin(), _groups.end(), free_ptr<Delete2ndPairObject<> >());
		_groups.clear();
		_fp.clear_flag(FieldTrait::present);
		_pos.clear();
	}

	unsigned decode(const f8String& from, const unsigned offset);
	unsigned decode_group(const unsigned short fnum, const f8String& from, const unsigned offset);
	unsigned encode(std::ostream& to);
	unsigned encode_group(const unsigned short fnum, std::ostream& to);
	unsigned check_positions();

	bool add_field(BaseField *what)
	{
		const unsigned short fnum(what->_fnum);
		if (_fp.has(fnum))
		{
			add_field(fnum, _fp.getPos(fnum), what);
			return true;
		}
		return false;
	}
	bool operator+=(BaseField *what) { return add_field(what); }

	template<typename T>
	GroupBase *find_group() { return find_group(T::get_fnum()); }
	GroupBase *find_group(const unsigned short fnum)
	{
		Groups::const_iterator gitr(_groups.find(fnum));
		return gitr != _groups.end() ? gitr->second : 0;
	}

	void add_group(GroupBase *what)
		{ _groups.insert(Groups::value_type(what->_fnum, what)); }
	void operator+=(GroupBase *what) { add_group(what); }

	virtual void print(std::ostream& os);
	virtual void print_group(const unsigned short fnum, std::ostream& os);
	friend class Message;
};

//-------------------------------------------------------------------------------------------------
class Message : public MessageBase
{
protected:
	MessageBase *_header, *_trailer;

public:
	template<typename InputIterator>
	Message(const F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: MessageBase(ctx, msgType, begin, end), _header(ctx._mk_hdr()), _trailer(ctx._mk_trl()) {}

	virtual ~Message()
	{
		delete _header;
		delete _trailer;
	}

	MessageBase *Header() { return _header; }
	MessageBase *Trailer() { return _trailer; }

	unsigned decode(const f8String& from);
	unsigned encode(f8String& to);

	virtual void print(std::ostream& os);

	static unsigned calc_chksum(const f8String& from)
	{
		unsigned val(0);
		for (f8String::const_iterator itr(from.begin()); itr != from.end(); ++itr)
			val += *itr;
		return val % 256;
	}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_MESSAGE_HPP_

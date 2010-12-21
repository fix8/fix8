//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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

---------------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
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
#if defined POOLALLOC
typedef std::vector<MessageBase *, f8Allocator<MessageBase *> > GroupElement;
#else
typedef std::vector<MessageBase *> GroupElement;
#endif

//-------------------------------------------------------------------------------------------------
class GroupBase : public f8Base
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
	MessageBase *operator[](unsigned idx) { return idx < _msgs.size() ? _msgs[idx] : 0; }

	void clear(bool reuse=false)
	{
		std::for_each (_msgs.begin(), _msgs.end(), free_ptr<>());
		if (reuse)
			_msgs.clear();
	}

	friend class MessageBase;
};

#if defined POOLALLOC
typedef std::map<unsigned short, GroupBase *, std::less<unsigned short>,
	f8Allocator<std::pair<unsigned short, GroupBase *> > > Groups;
#else
typedef std::map<unsigned short, GroupBase *> Groups;
#endif

//-------------------------------------------------------------------------------------------------
class Router
{
public:
	virtual bool operator()(const Message *msg) const { return false; }
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
#if defined POOLALLOC
typedef std::map<unsigned short, BaseField *, std::less<unsigned short>,
	f8Allocator<std::pair<unsigned short, BaseField *> > > Fields;
typedef std::multimap<unsigned short, BaseField *, std::less<unsigned short>,
	f8Allocator<std::pair<unsigned short, BaseField *> > > Positions;
#else
typedef std::map<unsigned short, BaseField *> Fields;
typedef std::multimap<unsigned short, BaseField *> Positions;
#endif

class MessageBase : public f8Base
{
protected:
	static RegExp _elmnt;

	Fields _fields;
	FieldTraits _fp;
	Positions _pos;
	Groups _groups;
	const f8String& _msgType;
	const class F8MetaCntx& _ctx;

public:
	template<typename InputIterator>
	MessageBase(const class F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: _fp(begin, end), _msgType(msgType), _ctx(ctx) {}

	MessageBase(const MessageBase& from);
	MessageBase& operator=(const MessageBase& that);

	virtual ~MessageBase() { clear(); }

	void clear(bool reuse=false)
	{
		std::for_each (_fields.begin(), _fields.end(), free_ptr<Delete2ndPairObject<> >());
		std::for_each (_groups.begin(), _groups.end(), free_ptr<Delete2ndPairObject<> >());
		if (reuse)
		{
			_fields.clear();
			_groups.clear();
			_fp.clear_flag(FieldTrait::present);
			_pos.clear();
		}
	}

	unsigned decode(const f8String& from, const unsigned offset);
	unsigned decode_group(const unsigned short fnum, const f8String& from, const unsigned offset);
	unsigned encode(std::ostream& to);
	unsigned encode_group(const unsigned short fnum, std::ostream& to);
	unsigned check_positions();
	unsigned copy_legal(MessageBase *to, bool force=false) const;
	void check_set_rlm(BaseField *where);

	const f8String& get_msgtype() const { return _msgType; }

	void add_field(const unsigned short fnum, const unsigned pos, BaseField *what)
	{
		_fields.insert(Fields::value_type(fnum, what));
		_pos.insert(Positions::value_type(pos, what));
		_fp.set(fnum, FieldTrait::present);
	}

	void set(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present) { _fp.set(field, type); }

	bool add_field(BaseField *what)
	{
		const unsigned short fnum(what->_fnum);
		if (_fp.has(fnum))
		{
			add_field(fnum, _fp.getPos(fnum), what);
			return true;
		}
		throw InvalidField(fnum);
		return false;
	}
	bool operator+=(BaseField *what) { return add_field(what); }

	template<typename T>
	bool get(T& to) const
	{
		Fields::const_iterator fitr(_fields.find(to._fnum));
		if (fitr == _fields.end())
			return false;
		to.set(fitr->second->from<T>().get());
		return true;
	}

	template<typename T>
	bool operator()(T& to) const { return get(to); }

	Fields::const_iterator find_field(const unsigned short fnum) const { return _fields.find(fnum); }
	BaseField *get_field(const unsigned short fnum) const
	{
		Fields::const_iterator itr(_fields.find(fnum));
		return itr != _fields.end() ? itr->second : 0;
	}

	BaseField *replace(const unsigned short fnum, BaseField *with);

	template<typename T>
	GroupBase *find_group() { return find_group(T::get_fnum()); }
	GroupBase *find_group(const unsigned short fnum) const
	{
		Groups::const_iterator gitr(_groups.find(fnum));
		return gitr != _groups.end() ? gitr->second : 0;
	}

	void add_group(GroupBase *what)
		{ _groups.insert(Groups::value_type(what->_fnum, what)); }
	void operator+=(GroupBase *what) { add_group(what); }

	unsigned short getPos(const unsigned short field) const { return _fp.getPos(field); }

	virtual void print(std::ostream& os) const;
	virtual void print_group(const unsigned short fnum, std::ostream& os) const;
	friend std::ostream& operator<<(std::ostream& os, const MessageBase& what) { what.print(os); return os; }
	friend class Message;
};

//-------------------------------------------------------------------------------------------------
class Message : public MessageBase
{
protected:
	static RegExp _hdr, _tlr;
	MessageBase *_header, *_trailer;

public:
	template<typename InputIterator>
	Message(const F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: MessageBase(ctx, msgType, begin, end), _header(ctx._mk_hdr()), _trailer(ctx._mk_trl()) {}

	virtual ~Message() { delete _header; delete _trailer; }

	MessageBase *Header() const { return _header; }
	MessageBase *Trailer() const { return _trailer; }

	unsigned decode(const f8String& from);
	unsigned encode(f8String& to);
	Message *copy() const;

	virtual bool process(Router& rt) const { return (rt)(this); }
	virtual bool is_admin() const { return false; }

	static unsigned calc_chksum(const f8String& from, const unsigned offset=0, const int len=-1)
	{
		unsigned val(0);
		const char *eptr(from.c_str() + (len != -1 ? len + offset : from.size() - offset));
		for (const char *ptr(from.c_str() + offset); ptr < eptr; ++ptr)
			val += *ptr;
		return val % 256;
	}

	static const f8String fmt_chksum(const unsigned val);
	static Message *factory(const F8MetaCntx& ctx, const f8String& from);

	virtual void print(std::ostream& os) const;
	friend std::ostream& operator<<(std::ostream& os, const Message& what) { what.print(os); return os; }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_MESSAGE_HPP_

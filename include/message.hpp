//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
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
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO,   THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_MESSAGE_HPP_
#define _FIX8_MESSAGE_HPP_

#if defined HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif
#include <vector>

#if defined MSGRECYCLING
#include <tbb/atomic.h>
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class MessageBase;
class Message;
class Session;
typedef std::vector<MessageBase *> GroupElement;

//-------------------------------------------------------------------------------------------------
/// Abstract base class for all repeating groups
class GroupBase
{
	/// number of fields in this group
	unsigned short _fnum;
	/// vector of repeating messagebase groups
	GroupElement _msgs;

public:
	/*! ctor
	    \param fnum number of fields in this group */
	GroupBase(const unsigned short fnum) : _fnum(fnum) {}

	/// dtor
	virtual ~GroupBase() { clear(false); }

	/*! Create a new group element.
	  \return new message */
	virtual MessageBase *create_group() = 0;

	/*! Add a message to a repeating group
	  \param what message to add */
	void add(MessageBase *what) { _msgs.push_back(what); }

	/*! Add a message to a repeating group
	  \param what message to add */
	void operator+=(MessageBase *what) { add(what); }

	/*! Return number of elements in a repeating group.
	  \return number of elements */
	size_t size() const { return _msgs.size(); }

	/*! Provide array style index access to repeating groups
	  \param idx index of element to get
	  \return pointer to element or 0 if index out of range */
	MessageBase *operator[](const unsigned idx) const { return get_element(idx); }

	/*! Get an element from a group
	  \param idx index of element to get
	  \return pointer to element or 0 if index out of range */
	MessageBase *get_element(const unsigned idx) const { return idx < _msgs.size() ? _msgs[idx] : 0; }

	/*! Empty messages from container
	    \param reuse if true clear vector */
	void clear(bool reuse=true)
	{
		std::for_each (_msgs.begin(), _msgs.end(), free_ptr<>());
		if (reuse)
			_msgs.clear();
	}

	friend class MessageBase;
};

typedef
#if defined HAS_TR1_UNORDERED_MAP
	std::tr1::unordered_map
#else
	std::map
#endif
	<unsigned short, GroupBase *> Groups;

//-------------------------------------------------------------------------------------------------
/// Base class for inbound message routing
class Router
{
public:
	/*! Function operator; overloaded with each generated Fix message type.
	  \return true on success */
	virtual bool operator()(const Message *msg) const { return false; }
};

//-------------------------------------------------------------------------------------------------
/// Structure for framework generated message creation table
struct BaseMsgEntry
{
	Message *(*_create)();
	const char *_name, *_comment;
};

//-------------------------------------------------------------------------------------------------
#if defined PERMIT_CUSTOM_FIELDS
/// Custom field wrapper
class CustomFields
{
	typedef
#if defined HAS_TR1_UNORDERED_MAP
		std::tr1::unordered_map
#else
		std::map
#endif
		<unsigned, BaseEntry *> CustFields;
	CustFields _custFields;
	bool _cleanup;

public:
	/*! Ctor.
	    \param cleanup if true, delete BAseEntries on destruction */
	explicit CustomFields(bool cleanup=true) : _cleanup(cleanup) {}

	/// Dtor.
	virtual ~CustomFields()
	{
		if (_cleanup)
			std::for_each(_custFields.begin(), _custFields.end(), free_ptr<Delete2ndPairObject<> >());
	}

	/*! Add a new field.
	    \param fnum field number (tag)
	    \param be BaseEntry of field
	    \return true on success */
	bool add(unsigned fnum, BaseEntry *be)
		{ return _custFields.insert(CustFields::value_type(fnum, be)).second; }

	/*! Find a field.
	    \param fnum field number (tag)
	    \return pointer to BaseEntry or 0 if not found */
	BaseEntry *find_ptr(unsigned fnum)
	{
		CustFields::const_iterator itr(_custFields.find(fnum));
		return itr == _custFields.end() ? 0 : itr->second;
	}
};
#endif

//-------------------------------------------------------------------------------------------------
/// Static metadata context class - one per FIX xml schema
struct F8MetaCntx
{
	const unsigned _version;

	/// Framework generated lookup table to generate Fix messages
	const GeneratedTable<const f8String, BaseMsgEntry>& _bme;
	/// Framework generated lookup table to generate Fix fields
	const GeneratedTable<unsigned, BaseEntry>& _be;
#if defined PERMIT_CUSTOM_FIELDS
	/// User supplied lookup table to generate Fix fields
	CustomFields *_ube;
	void set_ube(CustomFields *ube) { _ube = ube; }
#endif

	Message *(*_mk_hdr)(), *(*_mk_trl)();
	/// Fix header beginstring
	const f8String _beginStr;

	F8MetaCntx(const unsigned version, const GeneratedTable<const f8String, BaseMsgEntry>& bme,
		const GeneratedTable<unsigned, BaseEntry>& be, const f8String& bg) :
			 _version(version), _bme(bme), _be(be),
#if defined PERMIT_CUSTOM_FIELDS
			 _ube(),
#endif
			_mk_hdr(_bme.find_ptr("header")->_create),
			_mk_trl(_bme.find_ptr("trailer")->_create), _beginStr(bg) {}

	/// 4 digit fix version <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	const unsigned version() const { return _version; }
};

//-------------------------------------------------------------------------------------------------
typedef
#if defined HAS_TR1_UNORDERED_MAP
	std::tr1::unordered_map
#else
	std::map
#endif
	<unsigned short, BaseField *> Fields;

typedef std::multimap<unsigned short, BaseField *> Positions;

/// Base class for all fix messages
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

public:
	/*! Ctor.
	    \tparam InputIterator input iterator type
	    \param ctx reference to generated metadata
	    \param msgType - reference to Fix message type
	    \param begin - InputIterator pointing to begining of field trait table
	    \param end - InputIterator pointing to end of field trait table */
	template<typename InputIterator>
	MessageBase(const class F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: _fp(begin, end), _msgType(msgType), _ctx(ctx) {}

	/// Copy ctor.
	MessageBase(const MessageBase& from);
	/// Assignment operator
	MessageBase& operator=(const MessageBase& that);

	/// Dtor.
	virtual ~MessageBase() { clear(false); }

	/*! Empty messages from container.
	    \param reuse if true clear vector */
	virtual void clear(bool reuse=true)
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

	/*! Decode from string.
	    \param from source string
	    \param offset in bytes to decode from
	    \return number of bytes consumed */
	unsigned decode(const f8String& from, const unsigned offset);

	/*! Decode repeating group from string.
	    \param fnum repeating group fix field num (no...)
	    \param from source string
	    \param offset in bytes to decode from
	    \return number of bytes consumed */
	unsigned decode_group(const unsigned short fnum, const f8String& from, const unsigned offset);

	/*! Encode message to stream.
	    \param to stream to encode to
	    \return number of bytes encoded */
	unsigned encode(std::ostream& to);

	/*! Encode message to stream.
	    \param fnum repeating group fix field num (no...)
	    \param to stream to encode to
	    \return number of fields encoded */
	unsigned encode_group(const unsigned short fnum, std::ostream& to);

	unsigned check_positions();

	/*! Copy all fields from this message to 'to' where the field is legal for 'to' and it is not already present in 'to'; includes repeating groups.
	    \param to target message
	    \param force if true copy all fields regardless, replacing any existing, adding any new
	    \return number of fields copied */
	unsigned copy_legal(MessageBase *to, bool force=false) const;

	/*! Check that this field has the realm (domain) pointer set; if not then set.
	    \param where field to check */
	void check_set_rlm(BaseField *where)
	{
		if (!where->_rlm)
		{
			const BaseEntry *tbe(_ctx._be.find_ptr(where->_fnum));
			if (tbe && tbe->_rlm)
				where->_rlm = tbe->_rlm;	// populate realm;
		}
	}

	/*! Get the message type as a string.
	    \return the Fix message type as a string */
	const f8String& get_msgtype() const { return _msgType; }

	/*! Add fix field to this message.
	    \param fnum field tag
	    \param pos position of field in message
	    \param what pointer to field */
	void add_field(const unsigned short fnum, const unsigned pos, BaseField *what)
	{
		if (_fp.get(fnum, FieldTrait::present)) // for now, silently replace duplicate
		{
			delete replace(fnum, what);
			return;
		}

		_fields.insert(Fields::value_type(fnum, what));
		_pos.insert(Positions::value_type(pos, what));
		_fp.set(fnum, FieldTrait::present);
	}

	/*! Set field attribute to given value.
	    \param field tag number
	    \param type fieldtrait type */
	void set(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present) { _fp.set(field, type); }

	/*! Add fix field to this message.
	    \param what pointer to field
		 \return true on success; throws InvalidField if not valid */
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

	/*! Add fix field to this message.
	    \param what pointer to field
	    \return true on success; throws InvalidField if not valid */
	bool operator+=(BaseField *what) { return add_field(what); }

	/*! Populate supplied field with value from message.
	    \tparam T type of field to get
	    \param to field to populate
	    \return true on success */
	template<typename T>
	bool get(T& to) const
	{
		Fields::const_iterator fitr(_fields.find(to._fnum));
		if (fitr == _fields.end())
			return false;
		to.set(fitr->second->from<T>().get());
		if (fitr->second->_rlm)
			to._rlm = fitr->second->_rlm;
		return true;
	}

	/*! Populate supplied field with value from message.
	    \tparam T type of field to get
	    \param to field to populate
	    \return true on success */
	template<typename T>
	bool operator()(T& to) const { return get(to); }

	/*! Check if a field is present in this message.
	    \param fnum field number
	    \return true is present */
	bool have(const unsigned short fnum) const { return _fp.get(fnum, FieldTrait::present); }

	/*! Check if a field is present in this message.
	    \param fnum field number
	    \return iterator to field or Fields::const_iterator::end */
	Fields::const_iterator find_field(const unsigned short fnum) const { return _fields.find(fnum); }

	/*! Check if a field is present in this message.
	    \param fnum field number
	    \return pointer to field or 0 if not found */
	BaseField *get_field(const unsigned short fnum) const
	{
		Fields::const_iterator itr(_fields.find(fnum));
		return itr != _fields.end() ? itr->second : 0;
	}

	/*! Get an iterator to fields present in this message.
	    \return iterator to the first field or Fields::const_iterator::end */
	Fields::const_iterator fields_begin() const { return _fields.begin(); }

	/*! Get an iterator to fields present in this message.
	    \return iterator to the last field + 1 */
	Fields::const_iterator fields_end() const { return _fields.end(); }

	/*! Replace a field value with another field value.
	    \param fnum field number
	    \param with field to replace with
	    \return pointer to original field or 0 if not found */
	BaseField *replace(const unsigned short fnum, BaseField *with);

	/*! Remove a field from this message.
	    \param fnum field number
	    \return pointer to original field or 0 if not found */
	BaseField *remove(const unsigned short fnum);

	/*! Find a group of a specified type.
	    \tparam T type of group to get
	    \return pointer to found group or 0 if not found */
	template<typename T>
	GroupBase *find_group() const { return find_group(T::_fnum); }

	/*! Find a group of a specified type.
	    \param fnum field number
	    \return pointer to found group or 0 if not found */
	GroupBase *find_group(const unsigned short fnum) const
	{
		Groups::const_iterator gitr(_groups.find(fnum));
		return gitr != _groups.end() ? gitr->second : 0;
	}

	/*! Add a repeating group to a message.
	    \param what pointer to group to add */
	void add_group(GroupBase *what)
		{ _groups.insert(Groups::value_type(what->_fnum, what)); }

	/*! Add a repeating group to a message.
	    \param what pointer to group to add */
	void operator+=(GroupBase *what) { add_group(what); }

	/*! Find the position of a field in a message.
	    \param field field number
	    \return position of field */
	unsigned short getPos(const unsigned short field) const { return _fp.getPos(field); }

	/*! Add a fieldtrait to the message.
	    \param what FieldTrait to add
	    \return true on success */
	bool add_trait(const FieldTrait& what) { return _fp.add(what); }

	/*! Add a range of fieldtraits to the message.
	    \tparam InputIterator input iterator type
	    \param begin first FieldTrait to add
	    \param end last + 1 fieldtrait to add */
	template<typename InputIterator>
	void add_trait(const InputIterator begin, const InputIterator end) { _fp.add(begin, end); }

	/*! Print the message to the specified stream.
	    \param os refererence to stream to print to */
	virtual void print(std::ostream& os) const;

	/*! Print the repeating group to the specified stream.
	    \param fnum field number
	    \param os refererence to stream to print to */
	virtual void print_group(const unsigned short fnum, std::ostream& os) const;

	/*! Inserter friend.
	    \param os stream to send to
	    \param what messagebase
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const MessageBase& what) { what.print(os); return os; }
	friend class Message;

	void print_fp(std::ostream& os) { os << _fp; }
};

//-------------------------------------------------------------------------------------------------
/// A complete Fix message with header, body and trailer
class Message : public MessageBase
{
#if defined MSGRECYCLING
	tbb::atomic<bool> _in_use;
#endif

protected:
	static RegExp _hdr, _tlr;
	MessageBase *_header, *_trailer;

public:
	/*! Ctor.
	    \tparam InputIterator input iterator type
	    \param ctx - reference to generated metadata
	    \param msgType - reference to Fix message type
	    \param begin - InputIterator pointing to begining of field trait table
	    \param end - InputIterator pointing to end of field trait table */
	template<typename InputIterator>
	Message(const F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const InputIterator end)
		: MessageBase(ctx, msgType, begin, end), _header(ctx._mk_hdr()), _trailer(ctx._mk_trl())
#if defined MSGRECYCLING
		{ _in_use = true; }

	/*! Indicate that this message is currently being used.
	    \param way if true, set inuse as true */
	void set_in_use(bool way=false) { _in_use = way; }

	/*! Indicate that this message is currently unused.
	    \return true if message is in use */
	bool get_in_use() const { return _in_use; }
#else
		{}
#endif
	/// Dtor.
	virtual ~Message() { delete _header; delete _trailer; }

	/*! Get a pointer to the message header.
	    \return pointer to header */
	MessageBase *Header() const { return _header; }

	/*! Get a pointer to the message trailer.
	    \return pointer to trailer */
	MessageBase *Trailer() const { return _trailer; }

	/*! Decode from string.
	    \param from source string
	    \return number of bytes consumed */
	unsigned decode(const f8String& from);

	/*! Encode message to stream.
	    \param to stream to encode to
	    \return number of bytes encoded */
	unsigned encode(f8String& to);

	/*! Clone this message.
	    \return pointer to copy of this message */
	Message *clone() const;

	/*! Initiate callback to appropriate process method from metadata.
	    \param rt reference to router instance
	    \return true if correctly processed */
	virtual bool process(Router& rt) const { return (rt)(this); }

	/*! Test whether this message is administrative.
	    \return true if administrative */
	virtual bool is_admin() const { return false; }

	/*! Empty messages from container
	    \param reuse if true clear vector */
	virtual void clear(bool reuse=true)
	{
		if (_header)
			_header->clear(reuse);
		if (_trailer)
			_trailer->clear(reuse);
		MessageBase::clear(reuse);
	}

	/*! Generate a checksum from an encoded buffer.
	    \param from source buffer encoded Fix message
	    \param offset starting offset
	    \param len maximum length
	    \return calculated checknum */
	static unsigned calc_chksum(const f8String& from, const unsigned offset=0, const int len=-1)
	{
		unsigned val(0);
		const char *eptr(from.c_str() + (len != -1 ? len + offset : from.size() - offset));
		for (const char *ptr(from.c_str() + offset); ptr < eptr; ++ptr)
			val += *ptr;
		return val % 256;
	}

	/*! Format a checksum into the required 3 digit, 0 padded string.
	    \param val checksum value
	    \return string containing formatted value */
	static const f8String fmt_chksum(const unsigned val);

	/*! Using supplied metatdata context and raw input buffer, decode and create appropriate Fix message
	    \param ctx reference to metadata object
	    \param from pointer to raw buffer containing Fix message
	    \param sess pointer to session
	    \param post_ctor post_ctor member of session
	    \return pointer to newly created Message (which will be a super class of the generated type) */
	static Message *factory(const F8MetaCntx& ctx, const char *from
#if defined PERMIT_CUSTOM_FIELDS
			, Session *sess=0, bool (Session::*post_ctor)(Message *msg)=0
#endif
		)
	{
		const f8String to(from);
		return factory(ctx, to
#if defined PERMIT_CUSTOM_FIELDS
			, sess, post_ctor
#endif
		);
	}

	/*! Using supplied metatdata context and raw input buffer, decode and create appropriate Fix message
	    \param ctx reference to metadata object
	    \param from reference to string raw buffer containing Fix message
	    \param sess pointer to session
	    \param post_ctor post_ctor member of session
	    \return pointer to newly created Message (which will be a super class of the generated type) */
	static Message *factory(const F8MetaCntx& ctx, const f8String& from
#if defined PERMIT_CUSTOM_FIELDS
			, Session *sess=0, bool (Session::*post_ctor)(Message *msg)=0
#endif
		);

	/*! Print the message to the specified stream.
	    \param os refererence to stream to print to */
	virtual void print(std::ostream& os) const;

	/*! Inserter friend.
	    \param os stream to send to
	    \param what message
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const Message& what) { what.print(os); return os; }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_MESSAGE_HPP_

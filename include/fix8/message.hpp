//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_MESSAGE_HPP_
#define FIX8_MESSAGE_HPP_

#include <vector>
#if defined FIX8_PREENCODE_MSG_SUPPORT
#include <array>
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class MessageBase;
class Message;
class Session;
class GroupBase;
using GroupElement = std::vector<MessageBase *>;

//-------------------------------------------------------------------------------------------------
using Groups = std::map<unsigned short, GroupBase *>;

/// Abstract base class for all repeating groups
class GroupBase
{
	/// field number
	unsigned short _fnum;
	/// vector of repeating messagebase groups
	GroupElement _msgs;

public:
	/*! ctor
	    \param fnum field number of this group */
	GroupBase(const unsigned short fnum) : _fnum(fnum) {}

	/// dtor
	virtual ~GroupBase() { clear(false); }

	/*! Create a new group element.
	    \param deepctor if true, construct nested groups
	  \return new message */
	virtual MessageBase *create_group(bool deepctor=true) const = 0;

	/*! Instantiate a new nested group element.
	    \param fnum field number of group to create
	  \return new message or nullptr if not a valid group for this group */
	virtual GroupBase *create_nested_group(unsigned short fnum) const { return nullptr; }

	/*! Add a message to a repeating group
	  \param what message to add */
	void add(MessageBase *what) { _msgs.push_back(what); }

	/*! Add a message to a repeating group
	  \param what message to add */
	void operator+=(MessageBase *what) { add(what); }

	/*! Add a message to repeating group
	    \param what pointer to field
	    \return reference to GroupBase on success */
	GroupBase& operator<<(MessageBase *what) { add(what); return *this; }

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
	MessageBase *get_element(const unsigned idx) const { return idx < _msgs.size() ? _msgs[idx] : nullptr; }

	/*! Empty messages from container
	    \param reuse if true clear vector */
	void clear(bool reuse=true);

	/*! Inserter friend.
	    \param os stream to send to
	    \param what messagebase
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const GroupBase& what);

	friend class MessageBase;
};

//-------------------------------------------------------------------------------------------------
/// Base class for inbound message routing
class Router
{
public:
	/// Dtor.
   virtual ~Router() {}

	/*! Function operator (const version); overloaded with each generated Fix message type.
	  \param msg const ptr to message to route
	  \return true on success */
	virtual bool operator()(const Message *msg) const { return false; }

	/*! Function operator; overloaded with each generated Fix message type.
	  \param msg const ptr to message to route
	  \return true on success */
	virtual bool operator()(const Message *msg) { return false; }
};

/// Structures for framework generated message creation and static trait interrogation
class Minst
{
	/*! Generate a message instantiator
	 */
	struct _gen
	{
		/*! Instantiate a message
			\tparam T type to instantiate
			\return new message */
		template<typename T>
		static Message *_make(bool deepctor) { return new T(deepctor); }

		/*! Instantiate a message cast to Message
			\tparam T type to instantiate
			\return new message */
		template<typename T, typename R>
		static Message *_make(bool deepctor) { return reinterpret_cast<Message *>(new T(deepctor)); }

#if defined FIX8_HAVE_EXTENDED_METADATA
		/*! SIOF static TraitHelper
			\tparam T type to instantiate
			\return ref to static TraitHelper */
		template<typename T, typename R = void>
		static const TraitHelper& _make_traits()
		{
			static const TraitHelper _helper { T::get_traits(), T::get_fieldcnt() };
			return _helper;
		}
#endif
	};

public:
	std::function<Message *(bool)> _do;
#if defined FIX8_HAVE_EXTENDED_METADATA
	const TraitHelper& (&_get_traits)();
#endif

	/*! Ctor
	  \tparam T type to instantiate */
   template<typename T, typename... args>
   Minst(Type2Type<T, args...>) : _do(_gen::_make<T, args...>)
#if defined FIX8_HAVE_EXTENDED_METADATA
		 , _get_traits(_gen::_make_traits<T, args...>)
#endif
	{}
};

/// Message instantiation table entry
struct BaseMsgEntry
{
   const Minst _create;
	const char *_name, *_comment;
};

//-------------------------------------------------------------------------------------------------
/// Field and Message metadata structures
//-------------------------------------------------------------------------------------------------
using c_str_compare = std::function<bool(const char *, const char *)>;
using msg_create = std::function<Message *(bool)>;
using MsgTable = GeneratedTable<const char *, BaseMsgEntry>;
using ReverseMsgTable = std::map<const char * const, const BaseMsgEntry *, c_str_compare>;
using FieldTable = GeneratedTable<unsigned, BaseEntry>;
using ReverseFieldTable = std::map<const char * const, const BaseEntry *, c_str_compare>;

//-------------------------------------------------------------------------------------------------
/// Static metadata context class - one per FIX xml schema
struct F8MetaCntx
{
	/// 4 digit fix version <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	const unsigned _version;

	/// Framework generated lookup table to generate Fix messages
	const MsgTable& _bme;
	/// Framework generated lookup table to generate Fix fields
	const FieldTable& _be;
	/// Framework generated component name table
	const char **_cn;
	/// Number of elements in Hash array
	const unsigned _flu_sz;
	/// Hash array for field lookup (built by ctor)
	const BaseEntry **_flu;
	/// References to the header and trailer create functions
	msg_create _mk_hdr, _mk_trl;
	/// Fix header beginstring
	const f8String _beginStr;
	/// Preamble length
	const size_t _preamble_sz;

	ReverseMsgTable _reverse_msgtable;
	ReverseFieldTable _reverse_fieldtable;
	static bool _comp(const char *p1, const char *p2) { return ::strcmp(p1, p2) < 0; }

	/*! Ctor.
	  \param version FIX version
	  \param bme Generated Message Table
	  \param be Generated Field Table
	  \param cn Component name table
	  \param bg BeginString */
	F8MetaCntx(const unsigned version, const MsgTable& bme, const FieldTable& be, const char **cn, const f8String& bg)
		: _version(version), _bme(bme), _be(be), _cn(cn),
		_flu_sz(_be.at(_be.size() - 1)->_key + 1), _flu(new const BaseEntry *[_flu_sz]),
		_mk_hdr(_bme.find_ref("header")._create._do), _mk_trl(_bme.find_ref("trailer")._create._do),
		_beginStr(bg), _preamble_sz(2 + _beginStr.size() + 1 + 3),
		_reverse_msgtable(_comp), _reverse_fieldtable(_comp)
	{
		if (_flu_sz == 1)
			throw f8Exception("F8MetaCntx initialisation incomplete");
		std::fill(_flu, _flu + _flu_sz, nullptr);
      for (unsigned offset(0); offset < _be.size(); ++offset)
			*(_flu + _be.at(offset)->_key) = &_be.at(offset)->_value;

		std::for_each(_be.begin(), _be.end(), [this](const FieldTable::Pair& pp)
			{ _reverse_fieldtable.emplace(std::make_pair(pp.second()._name, &pp.second())); });
		std::for_each(_bme.begin(), _bme.end(), [this](const MsgTable::Pair& pp)
			{ _reverse_msgtable.emplace(std::make_pair(pp.second()._name, &pp.second())); });
	}

	/// Dtor.
	~F8MetaCntx() { delete[] _flu; _flu = nullptr; }

	/*! Get the field BaseEntry object for this field number. Will use fast field index lookup.
	  \param fnum field to get
	  \return ptr to BaseEntry or 0 if not found */
	const BaseEntry *find_be(const unsigned short fnum) const
		{ return fnum < _flu_sz ? _flu[fnum] : nullptr; }

	/*! Get the field BaseEntry object for this field by tag. Reverse lookup.
	  \param fieldstr const char ptr to longname of field to get
	  \return ptr to BaseEntry or 0 if not found */
	const BaseEntry *reverse_find_be(const char *fieldstr) const
	{
		if (fieldstr && *fieldstr)
		{
			auto itr(_reverse_fieldtable.find(fieldstr));
			return itr != _reverse_fieldtable.cend() ? itr->second : nullptr;
		}
		return nullptr;
	}

	/*! Get the field number for this field by tag. Reverse lookup.
	  \param fieldstr const char ptr to longname of field to get
	  \return unsigned short field number */
	unsigned short reverse_find_fnum(const char *fieldstr) const
	{
		if (fieldstr && *fieldstr)
		{
			auto itr(_reverse_fieldtable.find(fieldstr));
			return itr != _reverse_fieldtable.cend() ? itr->second->_fnum : 0;
		}
		return 0;
	}

	/*! Create a new field of the tag type passed, and from the raw string given.
	  \param fnum FIX field number
	  \param from const char ptr to string containing value to construct from
	  \return ptr to BaseField or 0 if fnum not found */
	BaseField *create_field(unsigned short fnum, const char *from) const
	{
		const BaseEntry *be(find_be(fnum));
		return be ? be->_create._do(from, be->_rlm, -1) : nullptr;
	}

	/*! Create a new field of the tag type passed, and from the raw string given.
	  \param tag const char ptr to longname of field to create
	  \param from const char ptr to string containing value to construct from
	  \return ptr to BaseField or 0 if fnum not found */
	BaseField *create_field(const char *tag, const char *from) const
	{
		const BaseEntry *be(reverse_find_be(tag));
		return be ? be->_create._do(from, be->_rlm, -1) : nullptr;
	}

	/*! Get the message BaseMsgEntry object for this message.
	  \param tag const char ptr to name of message to get
	  \return ptr to BaseMsgEntry or 0 if not found */
	const BaseMsgEntry *find_bme(const char *tag) const { return _bme.find_ptr(tag); }

	/*! Get the message BaseMsgEntry object for this message. Reverse lookup.
	  \param msgstr const char ptr to longname of message to get
	  \return ptr to BaseMsgEntry or 0 if not found */
	const BaseMsgEntry *reverse_find_bme(const char *msgstr) const
	{
		auto itr(_reverse_msgtable.find(msgstr));
		return itr != _reverse_msgtable.cend() ? itr->second : nullptr;
	}

	/*! Create a new message from longname
	  \param tag const char ptr to tag of message
	  \param deepctor if true, do deep message construction
	  \return ptr to Message or 0 if tag not found */
	Message *create_msg_from_longname(const char *tag, bool deepctor=true) const
	{
		const BaseMsgEntry *bme(reverse_find_bme(tag));
		return bme ? bme->_create._do(deepctor) : nullptr;
	}

	/*! Create a new message of the tag type passed.
	  \param tag const char ptr to tag of message
	  \param deepctor if true, do deep message construction
	  \return ptr to Message or 0 if tag not found */
	Message *create_msg(const char *tag, bool deepctor=true) const
	{
		const BaseMsgEntry *bme(find_bme(tag));
		return bme ? bme->_create._do(deepctor) : nullptr;
	}

	/*! 4 digit fix version <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	  \return version */
	unsigned version() const { return _version; }

	/*! Get fix header beginstring
	  \return beginstring */
	const f8String& get_beginStr() const { return _beginStr; }

#if defined FIX8_HAVE_EXTENDED_METADATA
	//----------------------------------------------------------------------------------------------
	// The following methods can be used to iterate and interrogate static traits
	//----------------------------------------------------------------------------------------------
	using const_iterator = const FieldTrait*;
	using const_internal_result = std::pair<const_iterator, const_iterator>;
	static const_iterator find(unsigned short fnum, const TraitHelper& from)
	{
		const const_internal_result res(std::equal_range (from._traits, from._traits + from._fieldcnt,
			FieldTrait(fnum), FieldTrait::Compare()));
		return res.first != res.second ? res.first : nullptr;
	}

	static const_iterator begin(const TraitHelper& from) { return from._traits; }
	static const_iterator end(const TraitHelper& from) { return from._traits + from._fieldcnt; }
#endif
};

//-------------------------------------------------------------------------------------------------
using Fields = std::map <unsigned short, BaseField *>;
using Positions = std::multimap<unsigned short, BaseField *>;

/// Base class for all fix messages
class MessageBase
{
	/// pass through for permissive mode
	f8String _unknown;

protected:
	Fields _fields;
	FieldTraits _fp;
	Positions _pos;
	Groups _groups;
	const f8String& _msgType;
	const F8MetaCntx& _ctx;

	/*! Extract length and message type from a header buffer
	    \param from source buffer
	    \param len length to extract to
	    \param mtype message type to extract to
	    \return number of bytes consumed */
	static unsigned extract_header(const f8String& from, char *len, char *mtype);

	/*! Extract chksum from a trailer buffer
	    \param from source buffer
	    \param chksum chksum to extract to
	    \return number of bytes consumed */
	static unsigned extract_trailer(const f8String& from, f8String& chksum);

	// used by the printer
	F8API static unsigned _tabsize;

public:
	/*! Ctor.
	    \tparam InputIterator input iterator type
	    \param ctx reference to generated metadata
	    \param msgType - reference to Fix message type
	    \param begin - InputIterator pointing to begining of field trait table
	    \param cnt - number of elements in field trait table
	    \param ftha - field trait hash array */
	template<typename InputIterator>
	MessageBase(const struct F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const size_t cnt,
		const FieldTrait_Hash_Array *ftha) : _fp(begin, cnt, ftha),
		_msgType(msgType), _ctx(ctx) {}

	/// Copy ctor.
	F8API MessageBase(const MessageBase& from);
	/// Assignment operator
	F8API MessageBase& operator=(const MessageBase& that);

	/// Dtor.
	virtual ~MessageBase() { clear(false); }

	/*! Empty messages from container.
	    \param reuse if true clear vector */
	virtual void clear(bool reuse=true)
	{
    std::for_each(_fields.begin(), _fields.end(), [](Fields::value_type& pp) { delete pp.second; pp.second = nullptr; });
    std::for_each(_groups.begin(), _groups.end(), [](Groups::value_type& pp) { delete pp.second; pp.second = nullptr; });
		if (reuse)
		{
			_fields.clear();
			//_groups.clear();
			_fp.clear_flag(FieldTrait::present);
			_pos.clear();
		}
	}

	/// empty the positions map
	void clear_positions() { _pos.clear(); }

	/*! Get the number of possible fields in this message
	  \return number of fields */
	size_t size() const { return _fp.size(); }

	/*! Decode from string.
	    \param from source string
	    \param offset in bytes to decode from
	    \param ignore bytes to ignore counting back from end of message
	    \param permissive_mode if true, ignore unknown fields
	    \return number of bytes consumed */
	F8API unsigned decode(const f8String& from, unsigned offset, unsigned ignore=0, bool permissive_mode=false);

	/*! Decode repeating group from string using nested group method
	    \param grpbase pointer to groupbase of holding object
	    \param fnum repeating group fix field num (no...)
	    \param from source string
	    \param s_offset in bytes to decode from
	    \param ignore bytes to ignore counting back from end of message
	    \return number of bytes consumed */
	unsigned decode_group(GroupBase *grpbase, const unsigned short fnum, const f8String& from,
		unsigned s_offset, unsigned ignore);

	/*! Encode message to stream.
	    \param to stream to encode to
	    \return number of bytes encoded */
	F8API size_t encode(std::ostream& to) const;

	/*! Encode message to buffer.
	    \param to buffer to encode to
	    \return number of bytes encoded */
	F8API size_t encode(char *to) const;

	/*! Encode group message to stream.
	    \param fnum repeating group fix field num (no...)
	    \param to stream to encode to
	    \return number of bytes encoded */
	F8API size_t encode_group(const unsigned short fnum, std::ostream& to) const;

	/*! Encode group message to buffer.
	    \param fnum repeating group fix field num (no...)
	    \param to buffer to encode to
	    \return number of bytes encoded */
	F8API size_t encode_group(const unsigned short fnum, char *to) const;

	/*! Instantiate a new nested group element.
	    \param fnum field number of group to create
	  \return new message */
	virtual GroupBase *create_nested_group(unsigned short fnum) const { return nullptr; }

	/*! Check to see if positions of fields are as required.
	  \return field number of field not in order, 0 if all ok */
	F8API unsigned check_positions();

	/*! Copy all fields from this message to 'to' where the field is legal for 'to' and
		  it is not already present in 'to'; includes nested repeating groups.
	    \param to target message
	    \param force if true copy all fields regardless, replacing any existing, adding any new
	    \return number of fields copied */
	F8API unsigned copy_legal(MessageBase *to, bool force=false) const;

	/*! Move all fields from this message to 'to' where the field is legal for 'to' and
		  it is not already present in 'to'; includes nested repeating groups.  Not thread safe.
		  Source message must be heap based. Source message is invalidated (but can be deleted).
	    \param to target message
	    \param force if true move all fields regardless, replacing any existing, adding any new
	    \return number of fields moved */
	F8API unsigned move_legal(MessageBase *to, bool force=false);

	/*! Copy _unknown string from this message to given message */
	void push_unknown(MessageBase *to) const
	{
		if (_unknown.size() && to)
			to->_unknown = _unknown;
	}

	/*! Check that this field has the realm (domain) pointer set; if not then set.
	    \param where field to check */
	void check_set_rlm(BaseField *where) const
	{
		if (!where->_rlm)
		{
			const BaseEntry *tbe(_ctx.find_be(where->_fnum));
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
	void add_field_decoder(const unsigned short fnum, const unsigned pos, BaseField *what)
	{
		_fields.insert({fnum, what});
		_pos.insert({pos, what});
	}

	/*! Add fix field to this message.
	    \param fnum field tag
		 \param itr hint iterator: set to itr of found element
	    \param pos position of field in message
	    \param what pointer to field
	    \param check if false, don't check for presence */
	void add_field(const unsigned short fnum, Presence::const_iterator itr, const unsigned pos, BaseField *what, bool check)
	{
		if (check && _fp.get(fnum, itr, FieldTrait::present)) // for now, silently replace duplicate
		{
			glout_debug << _msgType << " replacing field:" << fnum;
			delete replace(fnum, itr, what);
			return;
		}

		_fields.insert({fnum, what});
		_pos.insert({pos, what});
		_fp.set(fnum, itr, FieldTrait::present);
	}

	/*! Add fix field to this message.
	    \param fnum field tag
		 \param fitr hint iterator: set to itr of found element
	    \param pos position of field in message
	    \param what pointer to field
	    \param check if false, don't check for presence */
	void add_field(const unsigned short fnum, Fields::iterator fitr, const unsigned pos, BaseField *what, bool check=true)
	{
		Presence::const_iterator itr(_fp.get_presence().end());
		if (check && _fp.get(fnum, itr, FieldTrait::present)) // for now, silently replace duplicate
		{
			glout_debug << _msgType << " replacing field:" << fnum;
			delete replace(fnum, itr, what);
			return;
		}

		_fields.insert(fitr, {fnum, what});
		_pos.insert({pos, what});
		_fp.set(fnum, itr, FieldTrait::present);
	}

	/*! Add fix field to this message.
	    \param fnum field tag
	    \param pos position of field in message
	    \param what pointer to field
	    \param check if false, don't check for presence */
	void add_field(const unsigned short fnum, const unsigned pos, BaseField *what, bool check=true)
	{
		Presence::const_iterator itr(_fp.get_presence().end());
		if (check && _fp.get(fnum, itr, FieldTrait::present)) // for now, silently replace duplicate
		{
			glout_debug << _msgType << " replacing field:" << fnum;
			delete replace(fnum, itr, what);
			return;
		}

		_fields.insert({fnum, what});
		_pos.insert({pos, what});
		_fp.set(fnum, itr, FieldTrait::present);
	}

	/*! Set field attribute to given value.
	    \param field tag number
	    \param type fieldtrait type */
	void set(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present) { _fp.set(field, type); }

	/*! Add fix field to this message.
	    \tparam T field type
	    \param what pointer to field
		 \return true on success; throws InvalidField if not valid */
   template<typename T>
	bool add_field(T *what)
	{
		Presence::const_iterator itr(_fp.get_presence().end());
		if (_fp.has(T::get_field_id(), itr))
		{
			add_field(T::get_field_id(), itr, _fp.getPos(T::get_field_id(), itr), what, true);
			return true;
		}
		throw InvalidField(T::get_field_id());
		return false;
	}

	/*! Add fix field to this message.
	    \tparam T field type
	    \param what pointer to field
		 \return true on success; throws InvalidField if not valid */
	bool add_field(BaseField *what)
	{
		const unsigned short& fnum(what->_fnum);
		Presence::const_iterator itr(_fp.get_presence().end());
		if (_fp.has(fnum, itr))
		{
			add_field(fnum, itr, _fp.getPos(fnum, itr), what, true);
			return true;
		}
		throw InvalidField(fnum);
		return false;
	}

	Groups& get_groups() { return _groups; }

	/*! Add fix field to this message.
	    \tparam T field type
	    \param what pointer to field
	    \return true on success; throws InvalidField if not valid */
   template<typename T>
	bool operator+=(T *what) { return add_field(what); }

	/*! Add fix field to this message.
	    \tparam T field type
	    \param what pointer to field
	    \return reference to MessageBase on success */
   template<typename T>
	MessageBase& operator<<(T *what) { add_field(what); return *this; }

	/*! Populate supplied field with value from message.
	    \tparam T type of field to get
	    \param to field to populate
	    \return true on success */
	template<typename T>
	bool get(T& to) const
	{
		Fields::const_iterator fitr(_fields.find(T::get_field_id()));
		if (fitr == _fields.end())
			return false;
		to.set(fitr->second->from<T>().get());
		if (fitr->second->_rlm)
			to._rlm = fitr->second->_rlm;
		return true;
	}

	/*! Check if a field is present.
	    \tparam T type of field to get
	    \return true if present */
	template<typename T>
	bool has() const { return _fp.get(T::get_field_id()); }

	/*! Check if a field is legal in a message.
	    \param fnum field number
	    \return true if present */
	bool is_legal(unsigned short fnum) const { return _fp.has(fnum); }

	/*! Check if a field is legal.
	    \tparam T type of field to check
	    \return true if present */
	template<typename T>
	bool is_legal() const { return _fp.has(T::get_field_id()); }

	/*! Get a pointer to a field. Inplace, 0 copy.
	    \tparam T type of field to get
	    \return pointer to field or 0 if not present */
	template<typename T>
	const T *get() const
	{
		Fields::const_iterator fitr(_fields.find(T::get_field_id()));
		return fitr == _fields.cend() ? nullptr : &fitr->second->from<T>();
	}

	/*! Populate supplied field with value from message.
	    \tparam T type of field to get
	    \param to field to populate
	    \return true on success */
	template<typename T>
	bool operator()(T& to) const { return get(to); }

	/*! Get a pointer to a field. Inplace, 0 copy.
	    \tparam T type of field to get
	    \return pointer to field or 0 if not present */
	template<typename T>
	const T *operator()() const { return get<T>(); }

	/*! Check if a field is present in this message.
	    \param fnum field number
	    \return true is present */
	bool have(const unsigned short fnum) const { return _fp.get(fnum, FieldTrait::present); }

	/*! Check if a field is present in this message.
	    \param fnum field number
	    \return iterator to field or Fields::const_iterator::end */
	Fields::const_iterator find_field(const unsigned short fnum) const { return _fields.find(fnum); }

	/*! Search for field in this message.
	    \param fnum field number
	    \return pointer to field or 0 if not found */
	BaseField *get_field(const unsigned short fnum) const
	{
		auto itr(_fields.find(fnum));
		return itr != _fields.cend() ? itr->second : nullptr;
	}

	/*! Search for field by longname in this message.
	    \param tag field longname
	    \return pointer to field or 0 if not found */
	BaseField *get_field_by_name(const char *tag) const
	{
		auto itr(_fields.find(_ctx.reverse_find_fnum(tag)));
		return itr != _fields.cend() ? itr->second : nullptr;
	}

	/*! Get an iterator to fields present in this message.
	    \return iterator to the first field or Fields::const_iterator::end */
	Fields::const_iterator fields_begin() const { return _fields.begin(); }

	/*! Get an iterator to fields present in this message.
	    \return iterator to the last field + 1 */
	Fields::const_iterator fields_end() const { return _fields.end(); }

	/*! Replace a field value with another field value.
	    \param fnum field number
		 \param itr hint iterator: if end, set to itr of found element, if not end use it to locate element
	    \param with field to replace with
	    \return pointer to original field or 0 if not found */
	F8API BaseField *replace(const unsigned short fnum, Presence::const_iterator itr, BaseField *with);

	/*! Replace a field value with another field value.
	    \param fnum field number
	    \param with field to replace with
	    \return pointer to original field or 0 if not found */
	F8API BaseField *replace(const unsigned short fnum, BaseField *with);

	/*! Replace a group with another group.
	    \param fnum group field number
	    \param with group to replace with
	    \return pointer to original group or 0 if not found */
	F8API GroupBase *replace(const unsigned short fnum, GroupBase *with);

	/*! Remove a field from this message.
	    \param fnum field number
		 \param itr hint iterator: if end, set to itr of found element, if not end use it to locate element
	    \return pointer to original field or 0 if not found */
	F8API BaseField *remove(const unsigned short fnum, Presence::const_iterator itr);

	/*! Remove a field from this message.
	    \param fnum field number
	    \return pointer to original field or 0 if not found */
	F8API BaseField *remove(const unsigned short fnum);

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
		auto gitr(_groups.find(fnum));
		return gitr != _groups.cend() ? gitr->second : nullptr;
	}

	/*! Find a group of a specified type. If not found attempt to add.
	    \tparam T type of group to get
	    \param grpbase parent group (if group nested) or nullptr if no parent
	    \return pointer to found group or 0 if not found */
	template<typename T>
	GroupBase *find_add_group(GroupBase *grpbase=nullptr) { return find_add_group(T::_fnum, grpbase); }

	/*! Find a group of a specified type. If not found attempt to add.
	    \param fnum field number
	    \param grpbase parent group (if group nested) or nullptr if no parent
	    \return pointer to found group or 0 if not found */
	GroupBase *find_add_group(const unsigned short fnum, GroupBase *grpbase=nullptr)
	{
		auto gitr(_groups.find(fnum));
		if (gitr != _groups.end())
			return gitr->second;
		GroupBase *gb1(grpbase ? grpbase->create_nested_group(fnum) : create_nested_group(fnum));
		add_group(gb1);
		return gb1;
	}

	/*! Add a repeating group at the end of a message group. Assume key is not < last.
	    \tparam T type of grop being appended
	    \param what pointer to group to add */
	template<typename T>
	void append_group(T *what) { _groups.insert(_groups.end(), {T::_fnum, what}); }

	/*! Add a repeating group to a message.
	    \param what pointer to group to add */
	void add_group(GroupBase *what) { _groups.insert({what->_fnum, what}); }

	/*! Add a repeating group to a message.
	    \param what pointer to field
	    \return reference to MessageBase on success */
	MessageBase& operator<<(GroupBase *what) { add_group(what); return *this; }

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
	    \param cnt - number of elements in field trait table */
	template<typename InputIterator>
	void add_trait(const InputIterator begin, const size_t cnt) { _fp.add(begin, cnt); }

	/*! Print the message to the specified stream.
	    \param os refererence to stream to print to
	    \param depth nesting depth */
	F8API virtual void print(std::ostream& os, int depth=0) const;

	/*! Print the field specified by the field num from message to the specified stream.
	    \param fnum field number
	    \param os refererence to stream to print to */
	F8API virtual void print_field(const unsigned short fnum, std::ostream& os) const;

	/*! Print the repeating group to the specified stream.
	    \param fnum field number
	    \param os refererence to stream to print to
	    \param depth nesting depth */
	F8API virtual void print_group(const unsigned short fnum, std::ostream& os, int depth=0) const;

	/*! Get the FieldTraits
	   \return reference to FieldTraits object */
	const FieldTraits& get_fp() const { return _fp; }

	/*! Extract a tag/value element from a char buffer. ULL version.
	    \param from source buffer
	    \param sz size of string
	    \param tag tag to extract to
	    \param val value to extract to
	    \return number of bytes consumed */
	static unsigned extract_element(const char *from, const unsigned sz, char *tag, char *val)
	{
		enum { get_tag, get_value } state(get_tag);

		for (unsigned ii(0); ii < sz; ++ii)
		{
			switch (state)
			{
			case get_tag:
				if (!isdigit(from[ii]))
				{
					if (from[ii] != default_assignment_separator)
						return *val = *tag = 0;
					state = get_value;
				}
				else
					*tag++ = from[ii];
				break;
			case get_value:
				if (from[ii] == default_field_separator)
				{
					*val = *tag = 0;
					return ++ii;
				}
				*val++ = from[ii];
				break;
			}
		}
		return *val = *tag = 0;
	}

	/*! Extract a tag/value element from a char buffer.
	    \param from source buffer
	    \param sz size of string
	    \param tag tag to extract to
	    \param val value to extract to
	    \return number of bytes consumed */
	static unsigned extract_element(const char *from, const unsigned sz, f8String& tag, f8String& val)
	{
		enum { get_tag, get_value } state(get_tag);
		tag.clear();
		val.clear();

		for (unsigned ii(0); ii < sz; ++ii)
		{
			switch (state)
			{
			case get_tag:
				if (!isdigit(from[ii]))
				{
					if (from[ii] != default_assignment_separator)
						return 0;
					state = get_value;
				}
				else
					tag += from[ii];
				break;
			case get_value:
				if (from[ii] == default_field_separator)
					return ++ii;
				val += from[ii];
				break;
			}
		}
		return 0;
	}

	/*! Inserter friend.
	    \param os stream to send to
	    \param what messagebase
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const MessageBase& what) { what.print(os); return os; }
	friend class Message;

	/*! Set the tabsize used by the printer
	    \param tabsize number of spaces in a tab */
	static void set_tabsize (unsigned tabsize) { _tabsize = tabsize; }

	/*! get the tabsize used by the printer
	    \return tabsize of spaces in a tab */
	static unsigned get_tabsize () { return _tabsize; }

	/*! Determine if this repeating group count field has any elements (> 0)
	    \param bf Basefield *
	    \return true if has count */
	static bool has_group_count(const BaseField *bf) { return static_cast<const Field<int, 0> *>(bf)->get() > 0; }

	/*! Presence printer
	    \param os stream to send to */
	void print_fp(std::ostream& os) { os << _fp; }

	/*! Get pointer to begin_string Field; used by header/trailer.
	    \return Field */
	virtual begin_string *get_begin_string() { return nullptr; }

	/*! Get pointer to body_length Field; used by header/trailer.
	    \return Field */
	virtual body_length *get_body_length() { return nullptr; }

	/*! Get pointer to msg_type Field; used by header/trailer.
	    \return Field */
	virtual msg_type *get_msg_type() { return nullptr; }

	/*! Get pointer to check_sum Field; used by header/trailer.
	    \return Field */
	virtual check_sum *get_check_sum() { return nullptr; }

	/*! Get pass through fields (permissive mode only)
	    \return string of unknown pass through fields */
	const f8String& get_unknown() const { return _unknown; }
};

//-------------------------------------------------------------------------------------------------
inline void GroupBase::clear(bool reuse)
{
	std::for_each (_msgs.begin(), _msgs.end(), [](MessageBase *pp) { delete pp; });
	if (reuse)
		_msgs.clear();
}

//-------------------------------------------------------------------------------------------------
#if defined FIX8_CODECTIMING
struct codec_timings
{
	double _cpu_used;
	unsigned _msg_count;

	codec_timings() : _cpu_used(), _msg_count() {}
};

#endif

//-------------------------------------------------------------------------------------------------
#if defined FIX8_SIZEOF_UNSIGNED_LONG && FIX8_SIZEOF_UNSIGNED_LONG == 8
#ifndef _MSC_VER
constexpr unsigned long COLLAPSE_INT64(unsigned long x)
	   { return x + (x >> 8) + (x >> 16) + (x >> 24) + (x >> 32) + (x >> 40) + (x >> 48) + (x >> 56); }
#else
#define COLLAPSE_INT64(x) (x + (x >> 8) + (x >> 16) + (x >> 24) + (x >> 32) + (x >> 40) + (x >> 48) + (x >> 56))
#endif
#endif

//-------------------------------------------------------------------------------------------------
/// A complete Fix message with header, body and trailer
class Message : public MessageBase
{
#if defined FIX8_CODECTIMING
	static codec_timings _encode_timings, _decode_timings;
#endif

protected:
	MessageBase *_header, *_trailer;
	unsigned _custom_seqnum;
	bool _no_increment, _end_of_batch;
#if defined FIX8_RAW_MSG_SUPPORT
	mutable f8String _rawmsg;
	mutable int _begin_payload = -1;
	mutable unsigned _payload_len = 0;
#endif
#if defined FIX8_PREENCODE_MSG_SUPPORT
	mutable std::array<char, FIX8_MAX_MSG_LENGTH> _preencode;
	mutable size_t _preencode_len = 0;
#endif

public:
	/*! Ctor.
	    \tparam InputIterator input iterator type
	    \param ctx - reference to generated metadata
	    \param msgType - reference to Fix message type
	    \param begin - InputIterator pointing to begining of field trait table
	    \param cnt - number of elements in field trait table
		 \param ftha field trait hash array */
	template<typename InputIterator>
	Message(const F8MetaCntx& ctx, const f8String& msgType, const InputIterator begin, const size_t cnt,
		const FieldTrait_Hash_Array *ftha)
		: MessageBase(ctx, msgType, begin, cnt, ftha),_header(ctx._mk_hdr(true)),
		  _trailer(ctx._mk_trl(true)), _custom_seqnum(), _no_increment(), _end_of_batch(true)
	{}

	/// Dtor.
	virtual ~Message() { delete _header; delete _trailer; }

	/*! Get a pointer to the message header.
	    \return pointer to header */
	MessageBase *Header() const { return _header; }

	/*! Get a pointer to the message trailer.
	    \return pointer to trailer */
	MessageBase *Trailer() const { return _trailer; }

	/*! Get a pointer to the message header, non const version.
	    \return pointer to header */
	MessageBase *Header() { return _header; }

	/*! Get a pointer to the message trailer, non const version.
	    \return pointer to trailer */
	MessageBase *Trailer() { return _trailer; }

	/*! Decode from string.
	    \param from source string
	    \param offset in bytes to decode from
	    \param ignore bytes to ignore counting back from end of message
	    \param permissive_mode if true, ignore unknown fields
	    \return number of bytes consumed */
	unsigned decode(const f8String& from, unsigned offset=0, unsigned ignore=0, bool permissive_mode=false)
	{
		const unsigned hlen(_header->decode(from, offset, 0, permissive_mode));
		const unsigned blen(MessageBase::decode(from, hlen, 0, permissive_mode));
#if defined FIX8_RAW_MSG_SUPPORT
		_begin_payload = hlen;
		_payload_len = blen;
		_rawmsg = from;
#endif
		return _trailer->decode(from, blen, ignore, permissive_mode);
	}

	/*! Encode message to stream.
	    \param to stream to encode to
	    \return number of bytes encoded */
	F8API size_t encode(f8String& to) const;

	/*! Encode message to stream. Perform absolutely minimal copying of output buffer.
	    \param to pointer to pointer to buffer
	    \return number of bytes encoded; to ptr is updated with address of start of encoded message string */
	F8API size_t encode(char **to) const;

	/*! Clone this message. Performs a deep copy.
	    \return pointer to copy of this message */
	F8API Message *clone() const;

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

	/*! Generate a checksum from an encoded buffer, ULL version.
	    \param from char *buffer encoded Fix message
	    \param sz size of msg
	    \param offset starting offset
	    \param len maximum length
	    \return calculated checknum */
	static unsigned calc_chksum(const char *from, const size_t sz, const unsigned offset=0, const int len=-1)
	{
#if defined FIX8_SIZEOF_UNSIGNED_LONG && FIX8_SIZEOF_UNSIGNED_LONG == 8
		// steroid chksum algorithm by charles.cooper@lambda-tg.com
		// Basic strategy is to unroll the loop. normally adding in a loop
		// is slow because of the dependency, the cpu has to finish each loop
		// before it starts the next one and is unable to pipeline. one
		// way to get around this is to have multiple registers and have
		// multiple adds per loop, then consolidating at the end
		// as in unroll_checksum.
		// that is slow though because there is a lot of wasted space and
		// the cpu has to do a lot of adds.
		// key is that addition of a word is going to be faster
		// than addition of two chars, and also each word has 8 chars in it.
		// so we get 8 adds per loop. long1 + long2 is almost like
		// looping over two arrays of 8 chars and adding them element wise.
		// except for the overflow. so we have to keep track of the overflow
		// and get rid of it at the end.

		const unsigned long OVERFLOW_MASK (1UL << 8 | 1UL << 16 | 1UL << 24 | 1UL << 32 | 1UL << 40 | 1UL << 48 | 1UL << 56);
		unsigned long ret{}, overflow{};
		from += offset;
		const unsigned long elen (len != -1 ? len : sz);
		size_t ii{};
		for (; ii < (elen - elen % 8); ii += 8)
		{
			unsigned long expected_overflow((ret & OVERFLOW_MASK) ^ (OVERFLOW_MASK & *reinterpret_cast<const unsigned long*>(from + ii)));
			ret += *reinterpret_cast<const unsigned long*>(from + ii);
			overflow += (expected_overflow ^ ret) & OVERFLOW_MASK;
		}
		ret = COLLAPSE_INT64(ret);
		overflow = COLLAPSE_INT64(overflow);
		for (; ii < elen; ret += from[ii++]); // add up rest one by one
		return (ret - overflow) & 0xff;
#else
		// native chksum algorithm
		unsigned val(0);
		const char *eptr(from + (len != -1 ? len + offset : sz - offset));
		for (const char *ptr(from + offset); ptr < eptr; val += *ptr++);
		return val % 256;
#endif
	}

	/*! Generate a checksum from an encoded buffer, string version.
	    \param from source buffer encoded Fix message
	    \param offset starting offset
	    \param len maximum length
	    \return calculated checknum */
	static unsigned calc_chksum(const f8String& from, const unsigned offset=0, const int len=-1)
		{ return calc_chksum(from.c_str(), from.size(), offset, len); }

	/*! Format a checksum into the required 3 digit, 0 padded string.
	    \param val checksum value
	    \return string containing formatted value */
	static f8String fmt_chksum(const unsigned val)
	{
		char buf[4] { '0', '0', '0', 0 };
		itoa<unsigned>(val, buf + (val > 99 ? 0 : val > 9 ? 1 : 2), 10);
		return f8String(buf);
	}

	/*! Using supplied metatdata context and raw input buffer, decode and create appropriate Fix message
	    \param ctx reference to metadata object
	    \param from pointer to raw buffer containing one complete FIX message string only,
				no trailing or leading characters
	    \param no_chksum if true, do not perform chksum verification
	    \param permissive_mode if true, ignore unknown fields
	    \return pointer to newly created Message (which will be a super class of the generated type) */
	static Message *factory(const F8MetaCntx& ctx, const char *from, bool no_chksum=false, bool permissive_mode=false)
	{
		const f8String to(from);
		return factory(ctx, to, no_chksum, permissive_mode);
	}

	/*! Using supplied metatdata context and raw input buffer, decode and create appropriate Fix message
	    \param ctx reference to metadata object
	    \param from reference to string raw buffer containing one complete FIX message string only,
				no trailing or leading characters
	    \param no_chksum if true, do not perform chksum verification
	    \param permissive_mode if true, ignore unknown fields
	    \return pointer to newly created Message (which will be a super class of the generated type) */
	F8API static Message *factory(const F8MetaCntx& ctx, const f8String& from, bool no_chksum=false, bool permissive_mode=false);

	/*! Set the custom sequence number. Used to override and suppress automatic seqnum assignment.
	    \param seqnum the outbound sequence number to use for this message. */
	virtual void set_custom_seqnum(unsigned seqnum) { _custom_seqnum = seqnum; }

	/*! Get the custom sequence number.
	    \return seqnum the outbound sequence number to use for this message. */
	unsigned get_custom_seqnum() const { return _custom_seqnum; }

	/*! Set the no increment flag.
	    \param flag true means don't increment the seqnum after sending */
	virtual void set_no_increment(bool flag=true) { _no_increment = flag; }

	/*! Get the no increment flag.
	    \return value of _no_increment flag */
	virtual bool get_no_increment() const { return _no_increment; }

	/*! Get the end of batch flag
	    \return true or false */
	bool get_end_of_batch() const { return _end_of_batch; }

	/*! Set the end of batch flag
	    \param is_end_of_batch true or false */
	void set_end_of_batch(bool is_end_of_batch) { _end_of_batch = is_end_of_batch; }

	/*! Setup this message to allow it to be resused
	  This feature is experimental; do not use with pipelined mode */
	void setup_reuse()
	{
		if (_header)
		{
			_header->_fp.set(Common_BeginString, FieldTrait::suppress);
			_header->_fp.set(Common_BodyLength, FieldTrait::suppress);
			delete _header->remove(Common_MsgSeqNum);
		}
		if (_trailer)
			_trailer->_fp.set(Common_CheckSum, FieldTrait::suppress);
	}

	/*! Copy _unknown string from this message to given message */
	void push_unknown(Message *to) const
	{
		if (_header)
			_header->push_unknown(to->_header);
		MessageBase::push_unknown(to);
		if (_trailer)
			_trailer->push_unknown(to->_trailer);
	}

	/*! Search for field in this message (either header, body or trailer).
	    \param fnum field number
	    \return pointer to field or 0 if not found */
	BaseField *get_field_flattened(const unsigned short fnum) const
	{
		auto itr (_fields.find(fnum));
		return  itr != _fields.end() ? itr->second
				: (itr = _header->_fields.find(fnum)) != _header->_fields.end() ? itr->second
				: (itr = _trailer->_fields.find(fnum)) != _trailer->_fields.end() ? itr->second : nullptr;
	}

	/*! Search for field by longname in this message (either header, body or trailer).
	    \param tag field longname
	    \return pointer to field or 0 if not found */
	BaseField *get_field_by_name_flattened(const char *tag) const
	{
		return get_field_flattened(_ctx.reverse_find_fnum(tag));
	}

#if defined FIX8_RAW_MSG_SUPPORT
	/*! Get the raw FIX message that this message was decoded from
	    \return reference to FIX message string */
	const f8String& get_rawmsg() const { return _rawmsg; }

	/*! Get iterator to begin of message payload
	    \return const_iterator to start of payload */
	f8String::const_iterator begin_payload() const { return f8String::const_iterator(_rawmsg.data() + _begin_payload); }

	/*! Get iterator to end of message payload
	    \return const_iterator to end of payload */
	f8String::const_iterator end_payload() const { return f8String::const_iterator(_rawmsg.data() + _begin_payload + _payload_len); }

	/*! Get the payload length
	    \return payload length */
	unsigned get_payload_len() const { return _payload_len; }

	/*! Get the offset of payload begin
	    \return offset of payload begin */
	unsigned get_payload_begin() const { return _begin_payload; }
#endif
#if defined FIX8_PREENCODE_MSG_SUPPORT
	/*! Get the pre-encoded (prepared)
	    \return ptr to message or 0 if not prepared */
	const char *get_preencode_msg() const { return _preencode_len ? _preencode.data() : nullptr; }

	/*! Get the pre-encoded (prepared) mesage length
	    \return length */
	size_t get_preencode_len() const { return _preencode_len; }

	/*! Encode the payload to the prepare array
	    \return length encoded */
	size_t preencode() { return _preencode_len = MessageBase::encode(_preencode.data()); }
#endif

	/*! Print the message to the specified stream.
	    \param os refererence to stream to print to
	    \param depth not used */
	F8API virtual void print(std::ostream& os, int depth=0) const;

	/*! Inserter friend.
	    \param os stream to send to
	    \param what message
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const Message& what) { what.print(os); return os; }

#if defined FIX8_CODECTIMING
	F8API static void format_codec_timings(const f8String& md, std::ostream& ostr, codec_timings& tobj);
	F8API static void report_codec_timings(const f8String& tag);
#endif
};

//-------------------------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const GroupBase& what)
{
	for (const auto *pp : what._msgs)
		pp->print(os);
	return os;
}

} // FIX8

#endif // FIX8_MESSAGE_HPP_

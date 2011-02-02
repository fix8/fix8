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
#ifndef _F8_F8C_HPP_
#define _F8_F8C_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
struct Ctxt
{
	enum OutputFile { types_cpp, types_hpp, traits_cpp, classes_cpp, classes_hpp, router_hpp, count };
	typedef std::pair<std::pair<std::string, std::string>, std::ostream *> Output;
	Output _out[count];
	static const std::string _exts[count];
	unsigned _version;
	std::string _clname, _fixns, _systemns, _beginstr;
};

//-------------------------------------------------------------------------------------------------
class RealmObject
{
	bool _isRange;

protected:
	virtual bool comp(const RealmObject *p1, const RealmObject *p2) = 0;
	friend std::ostream& operator<<(std::ostream& os, RealmObject& what)
	{
		what.print(os);
		return os;
	}

	virtual void print(std::ostream& os) = 0;

public:
	RealmObject(bool isRange) : _isRange(isRange) {}
	virtual ~RealmObject() {}

	bool is_range() const { return _isRange; }
	static RealmObject *create(const std::string& from, FieldTrait::FieldType ftype, bool isRange=false);

	struct less
	{
		bool operator()(const RealmObject *p1, const RealmObject *p2)
			{ return const_cast<RealmObject*>(p1)->comp(p1, p2); }
	};
};

template <typename T>
class TypedRealm : public RealmObject
{
protected:
	T _obj;

public:
	TypedRealm(const T& from, bool isRange) : RealmObject(isRange), _obj(from) {}
	virtual void print(std::ostream& os) { os << _obj; }
	bool comp(const RealmObject *p1, const RealmObject *p2)
		{ return (static_cast<const TypedRealm<T>*>(p1))->_obj < static_cast<const TypedRealm<T>*>(p2)->_obj; }
};

struct StringRealm : public TypedRealm<std::string>
{
	StringRealm(const std::string& from, bool isRange) : TypedRealm<std::string>(from, isRange) {}
	void print(std::ostream& os) { os << '"' << _obj << '"'; }
};

struct CharRealm : public TypedRealm<char>
{
	CharRealm(const char& from, bool isRange) : TypedRealm<char>(from, isRange) {}
	void print(std::ostream& os) { os << '\'' << _obj << '\''; }
};

typedef std::map<RealmObject *, std::string, RealmObject::less> RealmMap;

//-------------------------------------------------------------------------------------------------
typedef StaticTable<std::string, FieldTrait::FieldType> BaseTypeMap;
typedef StaticTable<FieldTrait::FieldType, std::string> TypeToCPP;

struct FieldSpec
{
	static const BaseTypeMap _baseTypeMap;
	static const TypeToCPP _typeToCPP;

	std::string _name, _description, _comment;
	FieldTrait::FieldType _ftype;
	RealmBase::RealmType _dtype;
	unsigned _doffset;
	RealmMap *_dvals;

	FieldSpec(const std::string& name, FieldTrait::FieldType ftype=FieldTrait::ft_untyped)
		: _name(name), _ftype(ftype), _dtype(RealmBase::dt_set), _doffset(), _dvals() {}

	virtual ~FieldSpec()
	{
		if (_dvals)
			std::for_each(_dvals->begin(), _dvals->end(), free_ptr<Delete1stPairObject<> >());
		delete _dvals;
	}
};

//-------------------------------------------------------------------------------------------------
typedef std::map<unsigned, FieldSpec> FieldSpecMap;
typedef std::map<std::string, unsigned> FieldToNumMap;
typedef std::map<unsigned, FieldTraits> GroupMap;

//-------------------------------------------------------------------------------------------------
struct MessageSpec
{
	FieldTraits _fields;
	GroupMap _groups;
	std::string _name, _description, _comment;
	bool _is_admin;

	MessageSpec(const std::string& name, bool admin=false) : _name(name), _is_admin(admin) {}
	virtual ~MessageSpec() {}

	friend std::ostream& operator<<(std::ostream& os, const MessageSpec& what);
};

typedef std::map<const std::string, MessageSpec> MessageSpecMap;
typedef std::multiset<const FieldTrait *, FieldTrait::PosCompare> FieldTraitOrder;

//-------------------------------------------------------------------------------------------------
typedef MessageSpec ComponentSpec;
typedef MessageSpecMap ComponentSpecMap;

//-------------------------------------------------------------------------------------------------
enum comp_str
{
	cs_do_not_edit,
	cs_start_namespace,
	cs_end_namespace,
	cs_start_anon_namespace,
	cs_end_anon_namespace,
	cs_divider,
	cs_copyright,
	cs_copyright_short,
	cs_generated_includes,
	cs_header_preamble,
	cs_trailer_preamble,
};

typedef StaticTable<comp_str, std::string> CSMap;

} // FIX8

#endif // _F8_F8C_HPP_

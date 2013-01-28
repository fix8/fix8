//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _F8_F8C_HPP_
#define _F8_F8C_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// f8c compilation context.
struct Ctxt
{
	enum OutputFile { types_cpp, types_hpp, traits_cpp, classes_cpp,
		classes_hpp, router_hpp, session_hpp, count };
	typedef std::pair<std::pair<std::string, std::string>, std::ostream *> Output;
	Output _out[count];
	static const std::string _exts[count];
	unsigned _version;
	std::string _clname, _fixns, _systemns, _beginstr;
};

//-------------------------------------------------------------------------------------------------
/// f8c range or set domain realm.
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

/// f8c typed realm.
/*! \tparam T underlying realm type */
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

/// f8c string realm type.
struct StringRealm : public TypedRealm<std::string>
{
	StringRealm(const std::string& from, bool isRange) : TypedRealm<std::string>(from, isRange) {}
	void print(std::ostream& os) { os << '"' << _obj << '"'; }
};

/// f8c character realm type.
struct CharRealm : public TypedRealm<char>
{
	CharRealm(const char& from, bool isRange) : TypedRealm<char>(from, isRange) {}
	void print(std::ostream& os) { os << '\'' << _obj << '\''; }
};

typedef std::map<RealmObject *, std::string, RealmObject::less> RealmMap;

//-------------------------------------------------------------------------------------------------
typedef std::map<unsigned, struct FieldSpec> FieldSpecMap;
typedef std::map<std::string, unsigned> FieldToNumMap;
typedef std::map<unsigned, struct MessageSpec> GroupMap;

//-------------------------------------------------------------------------------------------------
typedef StaticTable<std::string, FieldTrait::FieldType> BaseTypeMap;
typedef StaticTable<FieldTrait::FieldType, std::string> TypeToCPP;

/// f8c internal field representation.
struct FieldSpec
{
	static const BaseTypeMap _baseTypeMap;
	static const TypeToCPP _typeToCPP;

	std::string _name, _description, _comment;
	FieldTrait::FieldType _ftype;
	RealmBase::RealmType _dtype;
	unsigned _doffset;
	RealmMap *_dvals;

	mutable bool _used;

	FieldSpec(const std::string& name, FieldTrait::FieldType ftype=FieldTrait::ft_untyped)
		: _name(name), _ftype(ftype), _dtype(RealmBase::dt_set), _doffset(), _dvals(), _used() {}

	virtual ~FieldSpec()
	{
		if (_dvals)
			std::for_each(_dvals->begin(), _dvals->end(), free_ptr<Delete1stPairObject<> >());
		delete _dvals;
	}

	void set_used() { _used = true; }
};

//-------------------------------------------------------------------------------------------------
/// f8c internal message representation.
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
enum comp_str
{
	cs_do_not_edit,
	cs_start_namespace,
	cs_end_namespace,
	cs_start_anon_namespace,
	cs_end_anon_namespace,
	cs_divider,
	cs_copyright,
	cs_copyright2,
	cs_copyright_short,
	cs_copyright_short2,
	cs_generated_includes,
	cs_header_preamble,
	cs_trailer_preamble,
};

typedef StaticTable<comp_str, std::string> CSMap;

//-----------------------------------------------------------------------------------------
inline int recover_line(const XmlElement& xf) { return xf.FindAttr("line", xf.GetLine()); }

} // FIX8

#endif // _F8_F8C_HPP_

//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <www@orbweb.org>
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
	enum OutputFile { types_cpp, types_hpp, classes_cpp, classes_hpp, count };
	typedef std::pair<std::string, scoped_ptr<std::ostream> > Output;
	Output _out[count];
};

//-------------------------------------------------------------------------------------------------
template<typename Key, typename Val>
struct StaticTable
{
	typedef typename std::map<Key, Val> TypeMap;
	typedef typename TypeMap::value_type TypePair;
	typedef Val NoValType;

	static const TypePair _valueTable[];
	static const TypeMap _valuemap;
	static const NoValType _noval;

	StaticTable() {}

	static const Val Find_Value(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}
	static const Val& Find_Value_Ref(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}

	static const size_t Get_Count() { return _valuemap.size(); }
	static const TypePair *Get_Table_End() { return _valueTable + sizeof(_valueTable)/sizeof(TypePair); }
};

//-------------------------------------------------------------------------------------------------
typedef StaticTable<std::string, FieldTrait::FieldType> BaseTypeMap;
typedef StaticTable<FieldTrait::FieldType, std::string> TypeToCPP;

typedef std::map<std::string, std::string> DomainMap;

struct FieldSpec
{
	static const BaseTypeMap _baseTypeMap;
	static const TypeToCPP _typeToCPP;

	std::string _name, _description, _domain;
	FieldTrait::FieldType _ftype;
	DomainMap *_dvals;

	FieldSpec(const std::string& name, FieldTrait::FieldType ftype=FieldTrait::ft_untyped)
		: _name(name), _ftype(ftype), _dvals() {}

	virtual ~FieldSpec() { delete _dvals; }
};

typedef std::map<unsigned, FieldSpec> FieldSpecMap;

//-------------------------------------------------------------------------------------------------
#if 0
struct MessageSpec
{
	static const BaseTypeMap _baseTypeMap;
	static const TypeToCPP _typeToCPP;

	std::string _name, _description, _domain;
	FieldTrait::FieldType _ftype;
	DomainMap *_dvals;

	MessageSpec(const std::string& name, FieldTrait::FieldType ftype=FieldTrait::ft_untyped)
		: _name(name), _ftype(ftype), _dvals() {}

	virtual ~FieldSpec() { delete _dvals; }
};

typedef std::map<unsigned, MessageSpec> MessageSpecMap;
#endif

//-------------------------------------------------------------------------------------------------
enum comp_str
{
	cs_do_not_edit,
	cs_start_namespace,
	cs_end_namespace,
	cs_generated_table_def,
	cs_divider,
	cs_copyright,
	cs_copyright_short,
	cs_fcreate_entry_hpp,
	cs_fcreate_entry_table,
	cs_fcreate_entry_cpp,
};

typedef StaticTable<comp_str, std::string> CSMap;
static const CSMap _csMap;

} // FIX8

#endif // _F8_F8C_HPP_

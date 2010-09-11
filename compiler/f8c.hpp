//-------------------------------------------------------------------------------------------------
#ifndef _F8_F8C_HPP_
#define _F8_F8C_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX {

typedef std::map<std::string, FieldTrait::FieldType> BaseTypeMap;
typedef std::map<std::string, FieldTrait::FieldSubType> TypeMap;
typedef std::map<FieldTrait::FieldType, std::string> CPPTypeMap;
typedef std::map<std::string, std::string> DomainMap;

struct FieldSpec
{
	static const BaseTypeMap::value_type _BaseTypes[];
	static const BaseTypeMap _basetypemap;
	static const TypeMap::value_type _Types[];
	static const TypeMap _typemap;
	static const CPPTypeMap::value_type _CPPTypes[];
	static const CPPTypeMap _cpptypemap;

	std::string _name;
	FieldTrait::FieldType _ftype;
	FieldTrait::FieldSubType _fstype;
	DomainMap *_dvals;

	FieldSpec(const std::string& name, FieldTrait::FieldType ftype)
		: _name(name), _ftype(ftype), _fstype(FieldTrait::fst_untyped), _dvals() {}
	FieldSpec(const std::string& name, FieldTrait::FieldSubType fstype)
		: _name(name), _ftype(FieldTrait::ft_untyped), _fstype(fstype), _dvals() {}

	virtual ~FieldSpec() { delete _dvals; }

	static const FieldTrait::FieldSubType Find_FieldSubType(std::string& tag)
	{
		TypeMap::const_iterator itr(_typemap.find(tag));
		return itr != _typemap.end() ? itr->second : FieldTrait::fst_untyped;
	}
	static const FieldTrait::FieldType Find_FieldType(std::string& tag)
	{
		BaseTypeMap::const_iterator itr(_basetypemap.find(tag));
		return itr != _basetypemap.end() ? itr->second : FieldTrait::ft_untyped;
	}
	static const std::string& Find_CPPType(FieldTrait::FieldType ft)
	{
		static const std::string _unknown("unknown");
		CPPTypeMap::const_iterator itr(_cpptypemap.find(ft));
		return itr != _cpptypemap.end() ? itr->second : _unknown;
	}
};

typedef std::map<unsigned, FieldSpec> FieldSpecMap;

} // FIX

#endif // _F8_F8C_HPP_

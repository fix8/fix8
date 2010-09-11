//-------------------------------------------------------------------------------------------------
#ifndef _F8_F8C_HPP_
#define _F8_F8C_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX {

template<typename Key, typename Val>
struct StaticTable
{
	typedef typename std::map<Key, Val> TypeMap;
	typedef typename TypeMap::value_type TypePair;
	typedef Val NoValType;

	static const TypePair _valueTable[];
	static const TypeMap _valuemap;
	static NoValType _noval;

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
typedef StaticTable<std::string, FieldTrait::FieldSubType> SubTypeMap;
typedef StaticTable<FieldTrait::FieldType, std::string> TypeToCPP;
typedef StaticTable<FieldTrait::FieldSubType, std::string> SubtypeToCPP;

typedef std::map<std::string, std::string> DomainMap;

struct FieldSpec
{
	static const BaseTypeMap _baseTypeMap;
	static const SubTypeMap _subTypeMap;
	static const TypeToCPP _typeToCPP;
	static const SubtypeToCPP _subtypeToCPP;

	std::string _name;
	FieldTrait::FieldType _ftype;
	FieldTrait::FieldSubType _fstype;
	DomainMap *_dvals;

	FieldSpec(const std::string& name, FieldTrait::FieldType ftype)
		: _name(name), _ftype(ftype), _fstype(FieldTrait::fst_untyped), _dvals() {}
	FieldSpec(const std::string& name, FieldTrait::FieldSubType fstype)
		: _name(name), _ftype(FieldTrait::ft_untyped), _fstype(fstype), _dvals() {}

	virtual ~FieldSpec() { delete _dvals; }
};

typedef std::map<unsigned, FieldSpec> FieldSpecMap;

} // FIX

#endif // _F8_F8C_HPP_

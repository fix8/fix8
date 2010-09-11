//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX_TRAITS_HPP_
#define _IF_FIX_TRAITS_HPP_

#include <set>

//-------------------------------------------------------------------------------------------------
namespace FIX {

struct FieldTrait
{
	unsigned short _fnum;
	enum FieldType { ft_untyped, ft_int, ft_float, ft_string, ft_char, ft_pattern };
	enum FieldSubType
	{
		fst_untyped,
		fst_Length, fst_TagNum, fst_SeqNum, fst_NumInGroup, fst_DayOfMonth,
		fst_Boolean,
		fst_Qty, fst_Price, fst_PriceOffset, fst_Amt, fst_Percentage,
		fst_MultipleCharValue, fst_MultipleStringValue, fst_Country, fst_Currency, fst_Exchange, fst_MonthYear,
		fst_UTCTimestamp, fst_UTCTimeOnly, fst_UTCDateOnly, fst_LocalMktDate, fst_TZTimeOnly, fst_TZTimestamp,
		fst_data, fst_XMLData,
		fst_Tenor, fst_Reserved100Plus, fst_Reserved1000Plus, fst_Reserved4000Plus
	} _fsubtype;

	unsigned short _pos;

	typedef std::map<FieldSubType, FieldType> FieldTypeMap;
	static const FieldTypeMap::value_type _subpair[];
	static const FieldTypeMap _fieldTypeMap;

	enum TraitTypes { mandatory, present, position, group, count };
	mutable std::bitset<count> _field_traits;

	FieldTrait(const unsigned short field, const FieldSubType fsubtype=fst_untyped,
		const unsigned short pos=0, bool ismandatory=false, bool isgroup=false, bool ispresent=false) :
		_fnum(field), _fsubtype(fsubtype), _pos(pos),
			_field_traits(ismandatory ? 1 : 0 | (ispresent ? 1 : 0) << present | 1 << position | (isgroup ? 1 : 0) << group) {}

	struct Compare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		bool operator()(const FieldTrait& p1, const FieldTrait& p2) const { return p1._fnum < p2._fnum; }
	};

	static const FieldType Find_FieldType(FieldSubType fst)
	{
		FieldTypeMap::const_iterator itr(_fieldTypeMap.find(fst));
		return itr != _fieldTypeMap.end() ? itr->second : ft_untyped;
	}
};

// which fields are required, which are present
class FieldTraits
{
	std::set<FieldTrait, FieldTrait::Compare> _presence;

public:
	template<typename InputIterator>
	FieldTraits(const InputIterator begin, const InputIterator end) : _presence(begin, end) {}
	FieldTraits() {}

	bool get(const unsigned short field, FieldTrait::TraitTypes type) const
	{
		std::set<FieldTrait, FieldTrait::Compare>::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.test(type) : false;
	}

	void set(const unsigned short field, FieldTrait::TraitTypes type)
	{
		std::set<FieldTrait, FieldTrait::Compare>::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.set(type, true);
	}

	bool isPresent(const unsigned short field) const { return get(field, FieldTrait::present); }
	bool isMandatory(const unsigned short field) const { return get(field, FieldTrait::mandatory); }
	bool isGroup(const unsigned short field) const { return get(field, FieldTrait::group); }
	unsigned short getPos(const unsigned short field) const
	{
		std::set<FieldTrait, FieldTrait::Compare>::const_iterator itr(_presence.find(field));
		return itr != _presence.end() && itr->_field_traits.test(FieldTrait::position) ? itr->_pos : 0;
	}
};

} // FIX

#endif // _IF_FIX_TRAITS_HPP_

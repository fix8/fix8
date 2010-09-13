//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX8_TRAITS_HPP_
#define _IF_FIX8_TRAITS_HPP_

#include <set>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

struct FieldTrait
{
	unsigned short _fnum;
	enum FieldType
	{
		ft_untyped,
		ft_int,
		ft_Length, ft_TagNum, ft_SeqNum, ft_NumInGroup, ft_DayOfMonth,
		ft_char,
		ft_Boolean,
		ft_float,
		ft_Qty, ft_Price, ft_PriceOffset, ft_Amt, ft_Percentage,
		ft_string,
		ft_MultipleCharValue, ft_MultipleStringValue, ft_Country, ft_Currency, ft_Exchange,
		ft_MonthYear, ft_UTCTimestamp, ft_UTCTimeOnly, ft_UTCDateOnly, ft_LocalMktDate, ft_TZTimeOnly, ft_TZTimestamp,
		ft_data, ft_XMLData,
		ft_pattern,
		ft_Tenor, ft_Reserved100Plus, ft_Reserved1000Plus, ft_Reserved4000Plus,
		ft_Language
	}
	_ftype;

	unsigned short _pos;

	enum TraitTypes { mandatory, present, position, group, count };
	mutable std::bitset<count> _field_traits;

	FieldTrait(const unsigned short field, const FieldType ftype=ft_untyped,
		const unsigned short pos=0, bool ismandatory=false, bool isgroup=false, bool ispresent=false) :
		_fnum(field), _ftype(ftype), _pos(pos),
			_field_traits(ismandatory ? 1 : 0 | (ispresent ? 1 : 0) << present | 1 << position | (isgroup ? 1 : 0) << group) {}

	struct Compare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		bool operator()(const FieldTrait& p1, const FieldTrait& p2) const { return p1._fnum < p2._fnum; }
	};
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

} // FIX8

#endif // _IF_FIX8_TRAITS_HPP_

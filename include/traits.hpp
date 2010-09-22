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
		ft_Length, ft_TagNum, ft_SeqNum, ft_NumInGroup, ft_DayOfMonth, ft_end_int=ft_DayOfMonth,
		ft_char,
		ft_Boolean, ft_end_char=ft_Boolean,
		ft_float,
		ft_Qty, ft_Price, ft_PriceOffset, ft_Amt, ft_Percentage, ft_end_float=ft_Percentage,
		ft_string,
		ft_MultipleCharValue, ft_MultipleStringValue, ft_Country, ft_Currency, ft_Exchange,
		ft_MonthYear, ft_UTCTimestamp, ft_UTCTimeOnly, ft_UTCDateOnly, ft_LocalMktDate, ft_TZTimeOnly, ft_TZTimestamp,
		ft_data, ft_XMLData,
		ft_pattern,
		ft_Tenor, ft_Reserved100Plus, ft_Reserved1000Plus, ft_Reserved4000Plus,
		ft_Language, ft_end_string=ft_Language
	}
	_ftype;

	struct TraitBase
	{
		const unsigned short _field;
		const FieldType _ftype;
		const unsigned short _pos;
		bool _ismandatory, _isgroup;
	};

	static bool is_int(const FieldType ftype) { return ft_int <= ftype && ftype <= ft_end_int; }
	static bool is_char(const FieldType ftype) { return ft_char <= ftype && ftype <= ft_end_char; }
	static bool is_string(const FieldType ftype) { return ft_string <= ftype && ftype <= ft_end_string; }
	static bool is_float(const FieldType ftype) { return ft_float <= ftype && ftype <= ft_end_float; }

	unsigned short _pos;	// is ordinal, ie 0=n/a, 1=1st, 2=2nd

	enum TraitTypes { mandatory, present, position, group, count };
	mutable ebitset<TraitTypes, unsigned short> _field_traits;

	FieldTrait(const unsigned short field, const FieldType ftype=ft_untyped,
		const unsigned short pos=0, bool ismandatory=false, bool isgroup=false, bool ispresent=false) :
		_fnum(field), _ftype(ftype), _pos(pos), _field_traits(ismandatory ? 1 : 0 | (ispresent ? 1 : 0) << present
		| (pos ? 1 : 0) << position | (isgroup ? 1 : 0) << group) {}

	FieldTrait(const TraitBase& tb) : _fnum(tb._field), _ftype(tb._ftype), _pos(tb._pos),
		_field_traits(tb._ismandatory ? 1 : 0 | (tb._pos ? 1 : 0) << position | (tb._isgroup ? 1 : 0) << group) {}

	struct Compare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		bool operator()(const FieldTrait& p1, const FieldTrait& p2) const { return p1._fnum < p2._fnum; }
	};
};

typedef std::set<FieldTrait, FieldTrait::Compare> Presence;

// which fields are required, which are present
class FieldTraits
{
	bool _hasMandatory, _hasGroup;
	Presence _presence;

public:
	template<typename InputIterator>
	FieldTraits(const InputIterator begin, const InputIterator end) : _hasMandatory(), _hasGroup(), _presence(begin, end) {}
	FieldTraits() : _hasMandatory(), _hasGroup() {}

	bool get(const unsigned short field, FieldTrait::TraitTypes type) const
	{
		std::set<FieldTrait, FieldTrait::Compare>::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.has(type) : false;
	}

	void set(const unsigned short field, FieldTrait::TraitTypes type)
	{
		std::set<FieldTrait, FieldTrait::Compare>::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.set(type);
	}

	bool add(const FieldTrait& what) { return _presence.insert(Presence::value_type(what)).second; }

	bool isPresent(const unsigned short field) const { return get(field, FieldTrait::present); }
	bool isMandatory(const unsigned short field) const { return get(field, FieldTrait::mandatory); }
	bool isGroup(const unsigned short field) const { return get(field, FieldTrait::group); }
	unsigned short getPos(const unsigned short field) const
	{
		std::set<FieldTrait, FieldTrait::Compare>::const_iterator itr(_presence.find(field));
		return itr != _presence.end() && itr->_field_traits.has(FieldTrait::position) ? itr->_pos : 0;
	}

	bool setHasMandatory(bool to=true) { return _hasMandatory = to; }
	bool setHasGroup(bool to=true) { return _hasGroup = to; }
	bool hasMandatory() const { return _hasMandatory; }
	bool hasGroup() const { return _hasGroup; }

	const Presence& get_presence() const { return _presence; }
};

} // FIX8

#endif // _IF_FIX8_TRAITS_HPP_

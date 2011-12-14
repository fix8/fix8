//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-11, David L. Dight <fix@fix8.org>
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

$Id$
$LastChangedDate$
$Rev$
$URL$

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_TRAITS_HPP_
#define _FIX8_TRAITS_HPP_

#include <set>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
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
		const unsigned short _field, _ftype, _pos;
		const unsigned _field_traits;
	};

	static bool is_int(const FieldType ftype) { return ft_int <= ftype && ftype <= ft_end_int; }
	static bool is_char(const FieldType ftype) { return ft_char <= ftype && ftype <= ft_end_char; }
	static bool is_string(const FieldType ftype) { return ft_string <= ftype && ftype <= ft_end_string; }
	static bool is_float(const FieldType ftype) { return ft_float <= ftype && ftype <= ft_end_float; }

	mutable unsigned short _pos;	// is ordinal, ie 0=n/a, 1=1st, 2=2nd
	mutable unsigned short _subpos;

	enum TraitTypes { mandatory, present, position, group, component, suppress, automatic, count };
	mutable ebitset<TraitTypes, unsigned short> _field_traits;

	FieldTrait(const unsigned short field, const FieldType ftype=ft_untyped, const unsigned short pos=0,
		bool ismandatory=false, bool isgroup=false, const unsigned subpos=0, bool ispresent=false) :
		_fnum(field), _ftype(ftype), _pos(pos), _subpos(subpos),
		_field_traits(ismandatory ? 1 : 0 | (ispresent ? 1 : 0) << present
		| (pos ? 1 : 0) << position | (isgroup ? 1 : 0) << group | (subpos ? 1 : 0) << component) {}

	FieldTrait(const TraitBase& tb) : _fnum(tb._field), _ftype(static_cast<FieldTrait::FieldType>(tb._ftype)),
		_pos(tb._pos), _subpos(), _field_traits(tb._field_traits | (tb._pos ? 1 : 0) << position) {}

	struct Compare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		bool operator()(const FieldTrait& p1, const FieldTrait& p2) const { return p1._fnum < p2._fnum; }
	};

	struct PosCompare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		bool operator()(const FieldTrait* p1, const FieldTrait* p2) const
			{ return p1->_pos < p2->_pos || (p1->_pos == p2->_pos && p1->_subpos < p2->_subpos); }
	};

	friend std::ostream& operator<<(std::ostream& os, const FieldTrait& what);
};

//-------------------------------------------------------------------------------------------------
typedef std::set<FieldTrait, FieldTrait::Compare> Presence;

// which fields are required, which are present
class FieldTraits
{
	Presence _presence;

public:
	template<typename InputIterator>
	FieldTraits(const InputIterator begin, const InputIterator end) : _presence(begin, end) {}
	FieldTraits() {}

	bool has(const unsigned short field) const
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end();
	}

	unsigned getval(const unsigned short field)
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.get() : 0;
	}

	bool get(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present) const
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.has(type) : false;
	}

	unsigned short find_missing(FieldTrait::TraitTypes type=FieldTrait::mandatory) const
	{
		for (Presence::const_iterator itr(_presence.begin()); itr != _presence.end(); ++itr)
			if ((itr->_field_traits & type) && (itr->_field_traits & FieldTrait::present) == 0)
				return itr->_fnum;
		return 0;
	}

	void set(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present)
	{
		Presence::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.set(type);
	}

	void clear(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present)
	{
		Presence::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.clear(type);
	}

	bool add(const FieldTrait& what) { return _presence.insert(Presence::value_type(what)).second; }
	void clear_flag(FieldTrait::TraitTypes type=FieldTrait::present)
		{ for (Presence::const_iterator itr(_presence.begin()); itr != _presence.end(); itr++->_field_traits.clear(type)); }

	bool is_present(const unsigned short field) const { return get(field, FieldTrait::present); }
	bool is_mandatory(const unsigned short field) const { return get(field, FieldTrait::mandatory); }
	bool is_group(const unsigned short field) const { return get(field, FieldTrait::group); }
	bool is_component(const unsigned short field) const { return get(field, FieldTrait::component); }
	unsigned short getPos(const unsigned short field) const
	{
		std::set<FieldTrait, FieldTrait::Compare>::const_iterator itr(_presence.find(field));
		return itr != _presence.end() && itr->_field_traits.has(FieldTrait::position) ? itr->_pos : 0;
	}

	const Presence& get_presence() const { return _presence; }

	friend std::ostream& operator<<(std::ostream& os, const FieldTraits& what);
};

} // FIX8

#endif // _FIX8_TRAITS_HPP_

//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
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
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_TRAITS_HPP_
#define _FIX8_TRAITS_HPP_

#include <set>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// FIX field traits - hold specific traits for each field.
struct FieldTrait
{
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
	};

	enum TraitTypes { mandatory, present, position, group, component, suppress, automatic, count };

	/*! Check if this FieldType is an int.
	  \param ftype field to check
	  \return true if an int */
	static bool is_int(const FieldType ftype) { return ft_int <= ftype && ftype <= ft_end_int; }

	/*! Check if this FieldType is a char.
	  \param ftype field to check
	  \return true if a char */
	static bool is_char(const FieldType ftype) { return ft_char <= ftype && ftype <= ft_end_char; }

	/*! Check if this FieldType is a string.
	  \param ftype field to check
	  \return true if a string */
	static bool is_string(const FieldType ftype) { return ft_string <= ftype && ftype <= ft_end_string; }

	/*! Check if this FieldType is a float.
	  \param ftype field to check
	  \return true if a float */
	static bool is_float(const FieldType ftype) { return ft_float <= ftype && ftype <= ft_end_float; }

	FieldTrait() {}

	FieldTrait(unsigned short fnum, unsigned ftype, unsigned short pos, short field_traits)
		: _fnum(fnum), _ftype(FieldType(ftype)), _pos(pos), _subpos(), _field_traits(field_traits | (pos ? 1 : 0) << position)  {}

	/*! Ctor.
	  \param field field num (tag number)
	  \param ftype field type
	  \param pos field position (in FIX message)
	  \param ismandatory true if mandatory
	  \param isgroup true if this is a group
	  \param subpos field sub-position (in FIX message)
	  \param ispresent true if field is present (should be false until set). */
	FieldTrait(const unsigned short field, const FieldType ftype=ft_untyped, const unsigned short pos=0,
		bool ismandatory=false, bool isgroup=false, const unsigned subpos=0, bool ispresent=false) :
		_fnum(field), _ftype(ftype), _pos(pos), _subpos(subpos),
		_field_traits(ismandatory ? 1 : 0 | (ispresent ? 1 : 0) << present
		| (pos ? 1 : 0) << position | (isgroup ? 1 : 0) << group | (subpos ? 1 : 0) << component) {}

	unsigned short _fnum;
	FieldType _ftype;
	mutable unsigned short _pos, _subpos;
	mutable ebitset<TraitTypes, unsigned short> _field_traits;

	/// Binary comparitor functor.
	struct Compare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		/*! Comparitor operator.
		  \param p1 lhs to compare
		  \param p2 rhs to compare
		  \return true if p1 < p2 */
		bool operator()(const FieldTrait& p1, const FieldTrait& p2) const { return p1._fnum < p2._fnum; }
	};

	/// Binary position comparitor functor.
	struct PosCompare : public std::binary_function<FieldTrait, FieldTrait, bool>
	{
		/*! Comparitor operator.
		  \param p1 lhs to compare
		  \param p2 rhs to compare
		  \return true if p1 position not equal to p2 position; will use subpos if necessary */
		bool operator()(const FieldTrait* p1, const FieldTrait* p2) const
			{ return p1->_pos < p2->_pos || (p1->_pos == p2->_pos && p1->_subpos < p2->_subpos); }
	};

	/*! Inserter friend.
	    \param os stream to send to
	    \param what FieldTrait
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const FieldTrait& what);
};

//-------------------------------------------------------------------------------------------------
typedef presorted_set<unsigned short, FieldTrait, FieldTrait::Compare> Presence;

/// A collection of FieldTraits for a message. Which fields are required, which are present.
class FieldTraits
{
	Presence _presence;

public:
	/*! Ctor.
	  \tparam InputIterator input iterator to construct from
	  \param begin start iterator to input
	  \param cnt number of elements to input */
	template<typename InputIterator>
	FieldTraits(const InputIterator begin, const size_t cnt) : _presence(begin, cnt) {}

	/// Ctor.
	FieldTraits() {}

	/*! Check if a field is present
	  \param field to check
	  \return true if present */
	bool has(const unsigned short field) const
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end();
	}

	/*! Get the traits for a field.
	  \param field to get
	  \return traits as an unsigned short */
	unsigned getval(const unsigned short field)
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.get() : 0;
	}

	/*! Get the number of possible fields
	  \return number of fields */
	size_t size() const { return _presence.size(); }

	/*! Check if a field has a specified trait.
	  \param field to check
	  \param type TraitType to check (default present)
	  \return true if field has trait */
	bool get(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present) const
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end() ? itr->_field_traits.has(type) : false;
	}

	/*! Find the first field that does not have the specified trait.
	  \param type TraitType to check (default mandatory)
	  \return field number of field, 0 if none */
	unsigned short find_missing(FieldTrait::TraitTypes type=FieldTrait::mandatory) const
	{
		for (Presence::const_iterator itr(_presence.begin()); itr != _presence.end(); ++itr)
			if ((itr->_field_traits & type) && (itr->_field_traits & FieldTrait::present) == 0)
				return itr->_fnum;
		return 0;
	}

	/*! Set a trait for a specified field.
	  \param field to set
	  \param type TraitType to set (default present) */
	void set(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present)
	{
		Presence::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.set(type);
	}

	/*! Clear a trait for a specified field.
	  \param field to set
	  \param type TraitType to set (default present) */
	void clear(const unsigned short field, FieldTrait::TraitTypes type=FieldTrait::present)
	{
		Presence::iterator itr(_presence.find(field));
		if (itr != _presence.end())
			itr->_field_traits.clear(type);
	}

	/*! Add a FieldTrait.
	  \param what TraitType to add
	  \return true on success (false already present) */
	bool add(const FieldTrait& what) { return _presence.insert(&what).second; }

	/*! Add from a range of traits.
	  \param begin start iterator to input
	  \param cnt number of elements to input */
	template<typename InputIterator>
	void add(const InputIterator begin, const size_t cnt) { _presence.insert(begin, begin + cnt); }

	/*! Clear a trait from all traits.
	  \param type TraitType to clear */
	void clear_flag(FieldTrait::TraitTypes type=FieldTrait::present)
		{ for (Presence::const_iterator itr(_presence.begin()); itr != _presence.end(); itr++->_field_traits.clear(type)); }

	/*! Check if a specified field has the present bit set (is present).
	  \param field field to check
	  \return true if present */
	bool is_present(const unsigned short field) const { return get(field, FieldTrait::present); }

	/*! Check if a specified field has the mandator bit set.
	  \param field field to check
	  \return true if mandatory set */
	bool is_mandatory(const unsigned short field) const { return get(field, FieldTrait::mandatory); }

	/*! Check if a specified field has the group bit set (is a group).
	  \param field field to check
	  \return true if a group */
	bool is_group(const unsigned short field) const { return get(field, FieldTrait::group); }

	/*! Check if a specified field has the component bit set (is a component).
	  \param field field to check
	  \return true if a component */
	bool is_component(const unsigned short field) const { return get(field, FieldTrait::component); }

	/*! Get the field position of a specified field.
	  \param field field to get
	  \return position of field, 0 if no pos or not found */
	unsigned short getPos(const unsigned short field) const
	{
		Presence::const_iterator itr(_presence.find(field));
		return itr != _presence.end() && itr->_field_traits.has(FieldTrait::position) ? itr->_pos : 0;
	}

	/*! Get the Presence set
	  \return the Presence set */
	const Presence& get_presence() const { return _presence; }

	/*! Inserter friend.
	    \param os stream to send to
	    \param what FieldTraits
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const FieldTraits& what);
};

} // FIX8

#endif // _FIX8_TRAITS_HPP_

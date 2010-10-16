#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <algorithm>
#include <bitset>

#include <strings.h>
#include <regex.h>

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
RegExp MessageBase::_elmnt("([0-9]+)=([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const f8String& from, const unsigned offset)
{
	RegMatch match;
	unsigned s_offset(offset);

	for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
	{
		f8String tag, val;
		_elmnt.SubExpr(match, from, tag, s_offset, 1);
		_elmnt.SubExpr(match, from, val, s_offset, 2);
		const unsigned tv(GetValue<unsigned>(tag));
		if (_fp.get(tv))
			throw DuplicateField(tv);;
		const BaseEntry *be(_ctx._be.find_ptr(tv));
		if (!be)
			throw InvalidField(tv);
		s_offset += match.SubSize();
		auto_ptr<BaseField> fld(be->_create(val, be->_dom));
		if (_fp.isGroup(tv))
			s_offset = decode_group(tv, from, s_offset);
		add_field(tv, ++pos, fld.release());
		_fp.set(tv);	// is present
	}

	const unsigned short missing(_fp.find_missing());
	if (missing)
		throw MissingMandatoryField(missing);

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode_group(const unsigned short fnum, const f8String& from, const unsigned offset)
{
	unsigned s_offset(offset);
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);

	bool ok(true);
	for (; ok && s_offset < from.size(); )
	{
		RegMatch match;
		auto_ptr<MessageBase> grp(grpbase->create_group());

		for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
		{
			f8String tag, val;
			_elmnt.SubExpr(match, from, tag, s_offset, 1);
			_elmnt.SubExpr(match, from, val, s_offset, 2);
			const unsigned tv(GetValue<unsigned>(tag));
			if (grp->_fp.get(tv))	// already present; next group?
				break;
			if (pos == 0 && grp->_fp.getPos(fnum) != 1)	// first field in group is mandatory
				throw MissingRepeatingGroupField(tv);
			s_offset += match.SubSize();
			const BaseEntry *be(_ctx._be.find_ptr(tv));
			if (!be)	// field not found in sub-group - end of repeats?
			{
				ok = false;
				break;
			}
			grp->add_field(tv, ++pos, be->_create(val, be->_dom));
			grp->_fp.set(tv);	// is present
		}

		const unsigned short missing(grp->_fp.find_missing());
		if (missing)
			throw MissingMandatoryField(missing);
		grpbase->_msgs.push_back(grp.release());
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::check_positions()
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
unsigned Message::decode(const f8String& from)
{
	_header = _ctx._create_header();
	unsigned offset(_header->decode(from, 0));
	offset = MessageBase::decode(from, offset);
	_trailer = _ctx._create_trailer();
	offset = _trailer->decode(from, offset);
	return 0;
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print(std::ostream& os)
{
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
		os << *itr->second << std::endl;
}

void Message::print(std::ostream& os)
{
	if (_header)
		_header->print(os);
	else
		os << "Null Header" << std::endl;
	MessageBase::print(os);
	if (_trailer)
		_trailer->print(os);
	else
		os << "Null Trailer" << std::endl;
}


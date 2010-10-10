#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
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

//-------------------------------------------------------------------------------------------------
RegExp MessageBase::_elmnt("([0-9]+)=([^]+)");

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const F8MetaCntx& ctx, const f8String& from, const unsigned offset)
{
	RegMatch match;
	unsigned s_offset(offset);
	while (s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3)
	{
		f8String tag, val;
		_elmnt.SubExpr(match, from, tag, s_offset, 1);
		_elmnt.SubExpr(match, from, val, s_offset, 2);
		s_offset += match.SubSize();
	}
	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::setupPositions()
{
	return 0;
}


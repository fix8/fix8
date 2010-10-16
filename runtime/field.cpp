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

#include <f8utils.hpp>
#include <f8exception.hpp>
#include <f8types.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
#if 0
template<const unsigned short field>
Field<UTCTimestamp, field>::Field (const std::string& from, const DomainBase *dom) : BaseField(field)
{
	if (from.size() == 6) // YYYYMM
	{
		_value.assign(GetValue<int>(from.substr(4)), GetValue<int>(from.substr(4, 2)), 1);
	}
	else if (from.size() == 8) // YYYYMMDD/WW
	{
	}
}

//-------------------------------------------------------------------------------------------------
#endif

} // namespace FIX8


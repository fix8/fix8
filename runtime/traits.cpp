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
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;

//-------------------------------------------------------------------------------------------------
#if 0
const FieldTrait::FieldTypeMap::value_type FieldTrait::_subpair[] =
{
	FieldTypeMap::value_type(FieldTrait::fst_Length, FieldTrait::ft_int),
	FieldTypeMap::value_type(FieldTrait::fst_TagNum, FieldTrait::ft_int),
	FieldTypeMap::value_type(FieldTrait::fst_SeqNum, FieldTrait::ft_int),
	FieldTypeMap::value_type(FieldTrait::fst_NumInGroup, FieldTrait::ft_int),
	FieldTypeMap::value_type(FieldTrait::fst_DayOfMonth, FieldTrait::ft_int),
	FieldTypeMap::value_type(FieldTrait::fst_Qty, FieldTrait::ft_float),
	FieldTypeMap::value_type(FieldTrait::fst_Price, FieldTrait::ft_float),
	FieldTypeMap::value_type(FieldTrait::fst_PriceOffset, FieldTrait::ft_float),
	FieldTypeMap::value_type(FieldTrait::fst_Amt, FieldTrait::ft_float),
	FieldTypeMap::value_type(FieldTrait::fst_Percentage, FieldTrait::ft_float),
	FieldTypeMap::value_type(FieldTrait::fst_Boolean, FieldTrait::ft_char),
	FieldTypeMap::value_type(FieldTrait::fst_MultipleCharValue, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_MultipleStringValue, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_Country, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_Currency, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_Exchange, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_MonthYear, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_UTCTimestamp, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_UTCTimeOnly, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_UTCDateOnly, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_LocalMktDate, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_TZTimeOnly, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_TZTimestamp, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_XMLData, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_data, FieldTrait::ft_string),
	FieldTypeMap::value_type(FieldTrait::fst_Tenor, FieldTrait::ft_pattern),
	FieldTypeMap::value_type(FieldTrait::fst_Reserved100Plus, FieldTrait::ft_pattern),
	FieldTypeMap::value_type(FieldTrait::fst_Reserved1000Plus, FieldTrait::ft_pattern),
	FieldTypeMap::value_type(FieldTrait::fst_Reserved4000Plus, FieldTrait::ft_pattern)
};

const FieldTrait::FieldTypeMap
	FieldTrait::_fieldTypeMap(FieldTrait::_subpair, FieldTrait::_subpair
		+ sizeof(FieldTrait::_subpair)/sizeof(FieldTrait::FieldTypeMap::value_type));
#endif

//-------------------------------------------------------------------------------------------------


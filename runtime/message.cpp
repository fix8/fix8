#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>
#include <bitset>

#include <field.hpp>
#include <traits.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;

//-------------------------------------------------------------------------------------------------
const FieldTrait MessageSubElements::header_ft[] =
{
	FieldTrait(1, FieldTrait::fst_Length, 1, true, false, false),
	FieldTrait(2, FieldTrait::fst_Length, 2, true, false, false),
	FieldTrait(3, FieldTrait::fst_Length, 3, true, false, false),
}, *MessageSubElements::header_ft_end(MessageSubElements::header_ft + sizeof(MessageSubElements::header_ft)/sizeof(FieldTrait));

const FieldTrait MessageSubElements::trailer_ft[] =
{
	FieldTrait(10, FieldTrait::fst_Length, 10, true, false, false),
}, *MessageSubElements::trailer_ft_end(MessageSubElements::trailer_ft + sizeof(MessageSubElements::trailer_ft)/sizeof(FieldTrait));


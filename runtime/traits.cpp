//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-----------------------------------------------------------------------------------------
#include <config.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <regex.h>

#include <f8utils.hpp>
#include <f8exception.hpp>
#include <f8types.hpp>
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


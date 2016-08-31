//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-16 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
const BaseTypeMap FieldSpec::_baseTypeMap
{
	{"INT", FieldTrait::ft_int},
	{"LENGTH", FieldTrait::ft_Length},
	{"TAGNUM", FieldTrait::ft_TagNum},
	{"SEQNUM", FieldTrait::ft_SeqNum},
	{"NUMINGROUP", FieldTrait::ft_NumInGroup},
	{"DAYOFMONTH", FieldTrait::ft_DayOfMonth},
	{"FLOAT", FieldTrait::ft_float},
	{"QTY", FieldTrait::ft_Qty},
	{"QUANTITY", FieldTrait::ft_Qty},
	{"PRICE", FieldTrait::ft_Price},
	{"PRICEOFFSET", FieldTrait::ft_PriceOffset},
	{"AMT", FieldTrait::ft_Amt},
	{"PERCENTAGE", FieldTrait::ft_Percentage},
	{"CHAR", FieldTrait::ft_char},
	{"BOOLEAN", FieldTrait::ft_Boolean},
	{"STRING", FieldTrait::ft_string},
	{"MULTIPLEVALUECHAR", FieldTrait::ft_MultipleCharValue},
	{"MULTIPLECHARVALUE", FieldTrait::ft_MultipleCharValue},
	{"MULTIPLESTRINGVALUE", FieldTrait::ft_MultipleStringValue},
	{"MULTIPLEVALUESTRING", FieldTrait::ft_MultipleStringValue},
	{"COUNTRY", FieldTrait::ft_Country},
	{"CURRENCY", FieldTrait::ft_Currency},
	{"EXCHANGE", FieldTrait::ft_Exchange},
	{"MONTHYEAR", FieldTrait::ft_MonthYear},
	{"UTCTIMESTAMP", FieldTrait::ft_UTCTimestamp},
	{"UTCTIME", FieldTrait::ft_UTCTimeOnly},
	{"UTCTIMEONLY", FieldTrait::ft_UTCTimeOnly},
	{"UTCDATE", FieldTrait::ft_UTCDateOnly},
	{"UTCDATEONLY", FieldTrait::ft_UTCDateOnly},
	{"LOCALMKTDATE", FieldTrait::ft_LocalMktDate},
	{"TZTIMEONLY", FieldTrait::ft_TZTimeOnly},
	{"TZTIMESTAMP", FieldTrait::ft_TZTimestamp},
	{"XMLDATA", FieldTrait::ft_XMLData},
	{"DATA", FieldTrait::ft_data},
	{"PATTERN", FieldTrait::ft_pattern},
	{"LANGUAGE", FieldTrait::ft_Language},
	{"TENOR", FieldTrait::ft_Tenor},
	{"RESERVED100PLUS", FieldTrait::ft_Reserved100Plus},
	{"RESERVED1000PLUS", FieldTrait::ft_Reserved1000Plus},
	{"RESERVED4000PLUS", FieldTrait::ft_Reserved4000Plus}
};

//-------------------------------------------------------------------------------------------------
const TypeToCPP FieldSpec::_typeToCPP
{
	{ FieldTrait::ft_int, { "int", "ft_int" } },
	{ FieldTrait::ft_Length, { "Length", "ft_Length" } },
	{ FieldTrait::ft_TagNum, { "TagNum", "ft_TagNum" } },
	{ FieldTrait::ft_SeqNum, { "SeqNum", "ft_SeqNum" } },
	{ FieldTrait::ft_NumInGroup, { "NumInGroup", "ft_NumInGroup" } },
	{ FieldTrait::ft_DayOfMonth, { "DayOfMonth", "ft_DayOfMonth" } },
	{ FieldTrait::ft_float, { "fp_type", "ft_float" } },	// either float or double
	{ FieldTrait::ft_Qty, { "Qty", "ft_Qty" } },
	{ FieldTrait::ft_Price, { "price", "ft_Price" } },
	{ FieldTrait::ft_PriceOffset, { "PriceOffset", "ft_PriceOffset" } },
	{ FieldTrait::ft_Amt, { "Amt", "ft_Amt" } },
	{ FieldTrait::ft_Percentage, { "Percentage", "ft_Percentage" } },
	{ FieldTrait::ft_char, { "char", "ft_char" } },
	{ FieldTrait::ft_Boolean, { "Boolean", "ft_Boolean" } },
	{ FieldTrait::ft_string, { "f8String", "ft_string" } },
	{ FieldTrait::ft_MultipleCharValue, { "MultipleCharValue", "ft_MultipleCharValue" } },
	{ FieldTrait::ft_MultipleStringValue, { "MultipleStringValue", "ft_MultipleStringValue" } },
	{ FieldTrait::ft_Country, { "country", "ft_Country" } },
	{ FieldTrait::ft_Currency, { "currency", "ft_Currency" } },
	{ FieldTrait::ft_Exchange, { "Exchange", "ft_Exchange" } },
	{ FieldTrait::ft_MonthYear, { "MonthYear", "ft_MonthYear" } },
	{ FieldTrait::ft_UTCTimestamp, { "UTCTimestamp", "ft_UTCTimestamp" } },
	{ FieldTrait::ft_UTCTimeOnly, { "UTCTimeOnly", "ft_UTCTimeOnly" } },
	{ FieldTrait::ft_UTCDateOnly, { "UTCDateOnly", "ft_UTCDateOnly" } },
	{ FieldTrait::ft_LocalMktDate, { "LocalMktDate", "ft_LocalMktDate" } },
	{ FieldTrait::ft_TZTimeOnly, { "TZTimeOnly", "ft_TZTimeOnly" } },
	{ FieldTrait::ft_TZTimestamp, { "TZTimestamp", "ft_TZTimestamp" } },
	{ FieldTrait::ft_XMLData, { "XMLData", "ft_XMLData" } },
	{ FieldTrait::ft_data, { "data", "ft_data" } },
	{ FieldTrait::ft_pattern, { "pattern", "ft_pattern" } },
	{ FieldTrait::ft_Tenor, { "Tenor", "ft_Tenor" } },
	{ FieldTrait::ft_Reserved100Plus, { "f8String", "ft_Reserved100Plus" } },
	{ FieldTrait::ft_Reserved1000Plus, { "f8String", "ft_Reserved1000Plus" } },
	{ FieldTrait::ft_Reserved4000Plus, { "f8String", "ft_Reserved4000Plus" } },
	{ FieldTrait::ft_Language, { "Language", "ft_Language" } },
};

//-------------------------------------------------------------------------------------------------
const CSMap _csMap
{
	{ cs_do_not_edit, "// *** f8c generated file: DO NOT EDIT! Created: "},
	{ cs_start_namespace, "namespace FIX8 {"},
	{ cs_end_namespace, "} // namespace FIX8"},
	{ cs_start_anon_namespace, "namespace {"},
	{ cs_end_anon_namespace, "} // namespace"},
	{ cs_generated_includes,
"#include <iostream>\n"
"#include <fstream>\n"
"#include <iomanip>\n"
"#include <sstream>\n"
"#include <vector>\n"
"#include <map>\n"
"#include <list>\n"
"#include <set>\n"
"#include <iterator>\n"
"#include <algorithm>\n"
"#include <cerrno>\n"
"#include <string.h>"},
	{ cs_divider,
"//-------------------------------------------------------------------------------------------------"},
{ cs_copyright,
"/*\n"
"\n"
"Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.\n"
"\n"
"Fix8 Open Source FIX Engine.\n"
"Copyright (C) 2010-"},
{ cs_copyright2,
" David L. Dight <fix@fix8.org>\n"
"\n"
"Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the\n"
"GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either\n"
"version 3 of the License, or (at your option) any later version.\n"
"\n"
"Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without\n"
"even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"\n"
"You should  have received a copy of the GNU Lesser General Public  License along with Fix8.\n"
"If not, see <http://www.gnu.org/licenses/>.\n"
"\n"
"*******************************************************************************************\n"
"*                Special note for Fix8 compiler generated source code                     *\n"
"*                                                                                         *\n"
"* Binary works  that are the results of compilation of code that is generated by the Fix8 *\n"
"* compiler  can be released  without releasing your  source code as  long as your  binary *\n"
"* links dynamically  against an  unmodified version of the Fix8 library.  You are however *\n"
"* required to leave the copyright text in the generated code.                             *\n"
"*                                                                                         *\n"
"*******************************************************************************************\n"
"\n"
"BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO\n"
"THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE\n"
"COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY\n"
"KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED\n"
"WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO\n"
"THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,\n"
"YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n"
"\n"
"IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT\n"
"HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED\n"
"ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR\n"
"CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT\n"
"NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR\n"
"THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH\n"
"HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\n"
"\n"
"*/\n"},
	{ cs_header_preamble,
"	begin_string *get_begin_string() { return _begin_string; };\n"
"	body_length *get_body_length() { return _body_length; };\n"
"	msg_type *get_msg_type() { return _msg_type; };\n\n"
"	void add_preamble()\n"
"	{\n"
"		add_field(Common_BeginString, 1, _begin_string, false);\n"
"		add_field(Common_BodyLength, 2, _body_length, false);\n"
"		add_field(Common_MsgType, 3, _msg_type, false);\n"
"	}"},
	{ cs_trailer_preamble,
"	check_sum *get_check_sum() { return _check_sum; };\n\n"
"	void add_preamble()\n"
"	{\n"
"		add_field(Common_CheckSum, 3, _check_sum, false);\n"
"	}"},
};

} // namespace FIX8
//-------------------------------------------------------------------------------------------------

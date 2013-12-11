//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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
const BaseTypeMap::TypePair bt_valueTable[] =
{
	BaseTypeMap::TypePair("INT", FieldTrait::ft_int),
	BaseTypeMap::TypePair("LENGTH", FieldTrait::ft_Length),
	BaseTypeMap::TypePair("TAGNUM", FieldTrait::ft_TagNum),
	BaseTypeMap::TypePair("SEQNUM", FieldTrait::ft_SeqNum),
	BaseTypeMap::TypePair("NUMINGROUP", FieldTrait::ft_NumInGroup),
	BaseTypeMap::TypePair("DAYOFMONTH", FieldTrait::ft_DayOfMonth),
	BaseTypeMap::TypePair("FLOAT", FieldTrait::ft_float),
	BaseTypeMap::TypePair("QTY", FieldTrait::ft_Qty),
	BaseTypeMap::TypePair("QUANTITY", FieldTrait::ft_Qty),
	BaseTypeMap::TypePair("PRICE", FieldTrait::ft_Price),
	BaseTypeMap::TypePair("PRICEOFFSET", FieldTrait::ft_PriceOffset),
	BaseTypeMap::TypePair("AMT", FieldTrait::ft_Amt),
	BaseTypeMap::TypePair("PERCENTAGE", FieldTrait::ft_Percentage),
	BaseTypeMap::TypePair("CHAR", FieldTrait::ft_char),
	BaseTypeMap::TypePair("BOOLEAN", FieldTrait::ft_Boolean),
	BaseTypeMap::TypePair("STRING", FieldTrait::ft_string),
	BaseTypeMap::TypePair("MULTIPLEVALUECHAR", FieldTrait::ft_MultipleCharValue),
	BaseTypeMap::TypePair("MULTIPLECHARVALUE", FieldTrait::ft_MultipleCharValue),
	BaseTypeMap::TypePair("MULTIPLESTRINGVALUE", FieldTrait::ft_MultipleStringValue),
	BaseTypeMap::TypePair("MULTIPLEVALUESTRING", FieldTrait::ft_MultipleStringValue),
	BaseTypeMap::TypePair("COUNTRY", FieldTrait::ft_Country),
	BaseTypeMap::TypePair("CURRENCY", FieldTrait::ft_Currency),
	BaseTypeMap::TypePair("EXCHANGE", FieldTrait::ft_Exchange),
	BaseTypeMap::TypePair("MONTHYEAR", FieldTrait::ft_MonthYear),
	BaseTypeMap::TypePair("UTCTIMESTAMP", FieldTrait::ft_UTCTimestamp),
	BaseTypeMap::TypePair("UTCTIME", FieldTrait::ft_UTCTimeOnly),
	BaseTypeMap::TypePair("UTCTIMEONLY", FieldTrait::ft_UTCTimeOnly),
	BaseTypeMap::TypePair("UTCDATE", FieldTrait::ft_UTCDateOnly),
	BaseTypeMap::TypePair("UTCDATEONLY", FieldTrait::ft_UTCDateOnly),
	BaseTypeMap::TypePair("LOCALMKTDATE", FieldTrait::ft_LocalMktDate),
	BaseTypeMap::TypePair("TZTIMEONLY", FieldTrait::ft_TZTimeOnly),
	BaseTypeMap::TypePair("TZTIMESTAMP", FieldTrait::ft_TZTimestamp),
	BaseTypeMap::TypePair("XMLDATA", FieldTrait::ft_XMLData),
	BaseTypeMap::TypePair("DATA", FieldTrait::ft_data),
	BaseTypeMap::TypePair("PATTERN", FieldTrait::ft_pattern),
	BaseTypeMap::TypePair("LANGUAGE", FieldTrait::ft_Language),
	BaseTypeMap::TypePair("TENOR", FieldTrait::ft_Tenor),
	BaseTypeMap::TypePair("RESERVED100PLUS", FieldTrait::ft_Reserved100Plus),
	BaseTypeMap::TypePair("RESERVED1000PLUS", FieldTrait::ft_Reserved1000Plus),
	BaseTypeMap::TypePair("RESERVED4000PLUS", FieldTrait::ft_Reserved4000Plus)
};
const BaseTypeMap FieldSpec::_baseTypeMap(bt_valueTable, sizeof(bt_valueTable)/sizeof(BaseTypeMap::TypePair),
		FieldTrait::ft_untyped);

//-------------------------------------------------------------------------------------------------
const TypeToCPP::TypePair tc_valueTable[] =
{
	TypeToCPP::TypePair(FieldTrait::ft_int, "int"),
	TypeToCPP::TypePair(FieldTrait::ft_Length, "Length"),
	TypeToCPP::TypePair(FieldTrait::ft_TagNum, "TagNum"),
	TypeToCPP::TypePair(FieldTrait::ft_SeqNum, "SeqNum"),
	TypeToCPP::TypePair(FieldTrait::ft_NumInGroup, "NumInGroup"),
	TypeToCPP::TypePair(FieldTrait::ft_DayOfMonth, "DayOfMonth"),
	TypeToCPP::TypePair(FieldTrait::ft_float, "double"),
	TypeToCPP::TypePair(FieldTrait::ft_Qty, "Qty"),
	TypeToCPP::TypePair(FieldTrait::ft_Price, "price"),
	TypeToCPP::TypePair(FieldTrait::ft_PriceOffset, "PriceOffset"),
	TypeToCPP::TypePair(FieldTrait::ft_Amt, "Amt"),
	TypeToCPP::TypePair(FieldTrait::ft_Percentage, "Percentage"),
	TypeToCPP::TypePair(FieldTrait::ft_char, "char"),
	TypeToCPP::TypePair(FieldTrait::ft_Boolean, "Boolean"),
	TypeToCPP::TypePair(FieldTrait::ft_string, "f8String"),
	TypeToCPP::TypePair(FieldTrait::ft_MultipleCharValue, "MultipleCharValue"),
	TypeToCPP::TypePair(FieldTrait::ft_MultipleStringValue, "MultipleStringValue"),
	TypeToCPP::TypePair(FieldTrait::ft_Country, "country"),
	TypeToCPP::TypePair(FieldTrait::ft_Currency, "currency"),
	TypeToCPP::TypePair(FieldTrait::ft_Exchange, "Exchange"),
	TypeToCPP::TypePair(FieldTrait::ft_MonthYear, "MonthYear"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCTimestamp, "UTCTimestamp"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCTimeOnly, "UTCTimeOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCDateOnly, "UTCDateOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_LocalMktDate, "LocalMktDate"),
	TypeToCPP::TypePair(FieldTrait::ft_TZTimeOnly, "TZTimeOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_TZTimestamp, "TZTimestamp"),
	TypeToCPP::TypePair(FieldTrait::ft_XMLData, "XMLData"),
	TypeToCPP::TypePair(FieldTrait::ft_data, "data"),
	TypeToCPP::TypePair(FieldTrait::ft_pattern, "pattern"),
	TypeToCPP::TypePair(FieldTrait::ft_Tenor, "Tenor"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved100Plus, "f8String"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved1000Plus, "f8String"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved4000Plus, "f8String"),
	TypeToCPP::TypePair(FieldTrait::ft_Language, "Language")
};
const TypeToCPP FieldSpec::_typeToCPP(tc_valueTable, sizeof(tc_valueTable)/sizeof(TypeToCPP::TypePair),
	"Unknown");

//-------------------------------------------------------------------------------------------------
const CSMap::TypePair cs_valueTable[] =
{
	CSMap::TypePair(cs_do_not_edit, "// *** f8c generated file: DO NOT EDIT! Created: "),
	CSMap::TypePair(cs_start_namespace, "namespace FIX8 {"),
	CSMap::TypePair(cs_end_namespace, "} // namespace FIX8"),
	CSMap::TypePair(cs_start_anon_namespace, "namespace {"),
	CSMap::TypePair(cs_end_anon_namespace, "} // namespace"),
	CSMap::TypePair(cs_generated_includes,
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
"#include <string.h>\n"
"#if defined HAS_TR1_UNORDERED_MAP\n"
"#include <tr1/unordered_map>\n"
"#endif"),
	CSMap::TypePair(cs_divider,
"//-------------------------------------------------------------------------------------------------"),
CSMap::TypePair(cs_copyright,
"/*\n"
"\n"
"Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.\n"
"\n"
"Fix8 Open Source FIX Engine.\n"
"Copyright (C) 2010-"),
CSMap::TypePair(cs_copyright2,
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
"*/\n"),
	CSMap::TypePair(cs_copyright_short, "Copyright (c) 2010-"),
	CSMap::TypePair(cs_copyright_short2, ", David L. Dight <fix@fix8.org>, All rights reserved."),
	CSMap::TypePair(cs_header_preamble,
"	begin_string *get_begin_string() { return _begin_string; };\n"
"	body_length *get_body_length() { return _body_length; };\n"
"	msg_type *get_msg_type() { return _msg_type; };\n\n"
"	void add_preamble()\n"
"	{\n"
"		add_field(Common_BeginString, 1, _begin_string, false);\n"
"		add_field(Common_BodyLength, 2, _body_length, false);\n"
"		add_field(Common_MsgType, 3, _msg_type, false);\n"
"	}"),
	CSMap::TypePair(cs_trailer_preamble,
"	check_sum *get_check_sum() { return _check_sum; };\n\n"
"	void add_preamble()\n"
"	{\n"
"		add_field(Common_CheckSum, 3, _check_sum, false);\n"
"	}"),
};

} // namespace FIX8
//-------------------------------------------------------------------------------------------------

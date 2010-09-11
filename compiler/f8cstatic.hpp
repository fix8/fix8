
//-----------------------------------------------------------------------------------------
template<>
const BaseTypeMap::TypePair BaseTypeMap::_valueTable[] =
{
	BaseTypeMap::TypePair("INT", FieldTrait::ft_int),
	BaseTypeMap::TypePair("FLOAT", FieldTrait::ft_float),
	BaseTypeMap::TypePair("CHAR", FieldTrait::ft_char),
	BaseTypeMap::TypePair("STRING", FieldTrait::ft_string),
	BaseTypeMap::TypePair("PATTERN", FieldTrait::ft_pattern)
};
template<>
const BaseTypeMap::TypeMap BaseTypeMap::_valuemap(BaseTypeMap::_valueTable, BaseTypeMap::Get_Table_End());
template<>
BaseTypeMap::NoValType BaseTypeMap::_noval(FieldTrait::ft_untyped);

//-----------------------------------------------------------------------------------------
template<>
const SubTypeMap::TypePair SubTypeMap::_valueTable[] =
{
	SubTypeMap::TypePair("LENGTH", FieldTrait::fst_Length),
	SubTypeMap::TypePair("TAGNUM", FieldTrait::fst_TagNum),
	SubTypeMap::TypePair("SEQNUM", FieldTrait::fst_SeqNum),
	SubTypeMap::TypePair("NUMINGROUP", FieldTrait::fst_NumInGroup),
	SubTypeMap::TypePair("DAYOFMONTH", FieldTrait::fst_DayOfMonth),
	SubTypeMap::TypePair("QTY", FieldTrait::fst_Qty),
	SubTypeMap::TypePair("PRICE", FieldTrait::fst_Price),
	SubTypeMap::TypePair("PRICEOFFSET", FieldTrait::fst_PriceOffset),
	SubTypeMap::TypePair("AMT", FieldTrait::fst_Amt),
	SubTypeMap::TypePair("PERCENTAGE", FieldTrait::fst_Percentage),
	SubTypeMap::TypePair("BOOLEAN", FieldTrait::fst_Boolean),
	SubTypeMap::TypePair("MULTIPLECHARVALUE", FieldTrait::fst_MultipleCharValue),
	SubTypeMap::TypePair("MULTIPLESTRINGVALUE", FieldTrait::fst_MultipleStringValue),
	SubTypeMap::TypePair("COUNTRY", FieldTrait::fst_Country),
	SubTypeMap::TypePair("CURRENCY", FieldTrait::fst_Currency),
	SubTypeMap::TypePair("EXCHANGE", FieldTrait::fst_Exchange),
	SubTypeMap::TypePair("MONTHYEAR", FieldTrait::fst_MonthYear),
	SubTypeMap::TypePair("UTCTIMESTAMP", FieldTrait::fst_UTCTimestamp),
	SubTypeMap::TypePair("UTCTIMEONLY", FieldTrait::fst_UTCTimeOnly),
	SubTypeMap::TypePair("UTCDATEONLY", FieldTrait::fst_UTCDateOnly),
	SubTypeMap::TypePair("LOCALMKTDATE", FieldTrait::fst_LocalMktDate),
	SubTypeMap::TypePair("TZTIMEONLY", FieldTrait::fst_TZTimeOnly),
	SubTypeMap::TypePair("TZTIMESTAMP", FieldTrait::fst_TZTimestamp),
	SubTypeMap::TypePair("XMLDATA", FieldTrait::fst_XMLData),
	SubTypeMap::TypePair("DATA", FieldTrait::fst_data),
	SubTypeMap::TypePair("TENOR", FieldTrait::fst_Tenor),
	SubTypeMap::TypePair("RESERVED100PLUS", FieldTrait::fst_Reserved100Plus),
	SubTypeMap::TypePair("RESERVED1000PLUS", FieldTrait::fst_Reserved1000Plus),
	SubTypeMap::TypePair("RESERVED4000PLUS", FieldTrait::fst_Reserved4000Plus)
};
template<>
const SubTypeMap::TypeMap SubTypeMap::_valuemap(SubTypeMap::_valueTable, SubTypeMap::Get_Table_End());
template<>
SubTypeMap::NoValType SubTypeMap::_noval(FieldTrait::fst_untyped);

//-----------------------------------------------------------------------------------------
template<>
const TypeToCPP::TypePair TypeToCPP::_valueTable[] =
{
	TypeToCPP::TypePair(FieldTrait::ft_int, "int"),
	TypeToCPP::TypePair(FieldTrait::ft_float, "double"),
	TypeToCPP::TypePair(FieldTrait::ft_char, "char"),
	TypeToCPP::TypePair(FieldTrait::ft_string, "std::string"),
	TypeToCPP::TypePair(FieldTrait::ft_pattern, "std::string")
};
template<>
const TypeToCPP::TypeMap TypeToCPP::_valuemap(TypeToCPP::_valueTable, TypeToCPP::Get_Table_End());
template<>
TypeToCPP::NoValType TypeToCPP::_noval("Unknown");

//-----------------------------------------------------------------------------------------
template<>
const SubtypeToCPP::TypePair SubtypeToCPP::_valueTable[] =
{
	SubtypeToCPP::TypePair(FieldTrait::fst_Length, "int"),
	SubtypeToCPP::TypePair(FieldTrait::fst_TagNum, "int"),
	SubtypeToCPP::TypePair(FieldTrait::fst_SeqNum, "int"),
	SubtypeToCPP::TypePair(FieldTrait::fst_NumInGroup, "int"),
	SubtypeToCPP::TypePair(FieldTrait::fst_DayOfMonth, "int"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Qty, "double"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Price, "double"),
	SubtypeToCPP::TypePair(FieldTrait::fst_PriceOffset, "double"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Amt, "double"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Percentage, "double"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Boolean, "char"),
	SubtypeToCPP::TypePair(FieldTrait::fst_MultipleCharValue, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_MultipleStringValue, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Country, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Currency, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Exchange, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_MonthYear, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_UTCTimestamp, "UTCTimestamp"),
	SubtypeToCPP::TypePair(FieldTrait::fst_UTCTimeOnly, "UTCTimeOnly"),
	SubtypeToCPP::TypePair(FieldTrait::fst_UTCDateOnly, "UTCDateOnly"),
	SubtypeToCPP::TypePair(FieldTrait::fst_LocalMktDate, "LocalMktDate"),
	SubtypeToCPP::TypePair(FieldTrait::fst_TZTimeOnly, "TZTimeOnly"),
	SubtypeToCPP::TypePair(FieldTrait::fst_TZTimestamp, "TZTimestamp"),
	SubtypeToCPP::TypePair(FieldTrait::fst_XMLData, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_data, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Tenor, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Reserved100Plus, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Reserved1000Plus, "std::string"),
	SubtypeToCPP::TypePair(FieldTrait::fst_Reserved4000Plus, "std::string")
};
template<>
const SubtypeToCPP::TypeMap SubtypeToCPP::_valuemap(SubtypeToCPP::_valueTable, SubtypeToCPP::Get_Table_End());
template<>
SubtypeToCPP::NoValType SubtypeToCPP::_noval("Unknown");

//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
const BaseTypeMap::value_type FieldSpec::_BaseTypes[] =
{
	BaseTypeMap::value_type("INT", FieldTrait::ft_int),
	BaseTypeMap::value_type("FLOAT", FieldTrait::ft_float),
	BaseTypeMap::value_type("CHAR", FieldTrait::ft_char),
	BaseTypeMap::value_type("STRING", FieldTrait::ft_string),
	BaseTypeMap::value_type("PATTERN", FieldTrait::ft_pattern)
};
const BaseTypeMap FieldSpec::_basetypemap(FieldSpec::_BaseTypes, FieldSpec::_BaseTypes + sizeof(FieldSpec::_BaseTypes)/sizeof(BaseTypeMap::value_type));

const TypeMap::value_type FieldSpec::_Types[] =
{
	TypeMap::value_type("LENGTH", FieldTrait::fst_Length),
	TypeMap::value_type("TAGNUM", FieldTrait::fst_TagNum),
	TypeMap::value_type("SEQNUM", FieldTrait::fst_SeqNum),
	TypeMap::value_type("NUMINGROUP", FieldTrait::fst_NumInGroup),
	TypeMap::value_type("DAYOFMONTH", FieldTrait::fst_DayOfMonth),
	TypeMap::value_type("QTY", FieldTrait::fst_Qty),
	TypeMap::value_type("PRICE", FieldTrait::fst_Price),
	TypeMap::value_type("PRICEOFFSET", FieldTrait::fst_PriceOffset),
	TypeMap::value_type("AMT", FieldTrait::fst_Amt),
	TypeMap::value_type("PERCENTAGE", FieldTrait::fst_Percentage),
	TypeMap::value_type("BOOLEAN", FieldTrait::fst_Boolean),
	TypeMap::value_type("MULTIPLECHARVALUE", FieldTrait::fst_MultipleCharValue),
	TypeMap::value_type("MULTIPLESTRINGVALUE", FieldTrait::fst_MultipleStringValue),
	TypeMap::value_type("COUNTRY", FieldTrait::fst_Country),
	TypeMap::value_type("CURRENCY", FieldTrait::fst_Currency),
	TypeMap::value_type("EXCHANGE", FieldTrait::fst_Exchange),
	TypeMap::value_type("MONTHYEAR", FieldTrait::fst_MonthYear),
	TypeMap::value_type("UTCTIMESTAMP", FieldTrait::fst_UTCTimestamp),
	TypeMap::value_type("UTCTIMEONLY", FieldTrait::fst_UTCTimeOnly),
	TypeMap::value_type("UTCDATEONLY", FieldTrait::fst_UTCDateOnly),
	TypeMap::value_type("LOCALMKTDATE", FieldTrait::fst_LocalMktDate),
	TypeMap::value_type("TZTIMEONLY", FieldTrait::fst_TZTimeOnly),
	TypeMap::value_type("TZTIMESTAMP", FieldTrait::fst_TZTimestamp),
	TypeMap::value_type("XMLDATA", FieldTrait::fst_XMLData),
	TypeMap::value_type("DATA", FieldTrait::fst_data),
	TypeMap::value_type("TENOR", FieldTrait::fst_Tenor),
	TypeMap::value_type("RESERVED100PLUS", FieldTrait::fst_Reserved100Plus),
	TypeMap::value_type("RESERVED1000PLUS", FieldTrait::fst_Reserved1000Plus),
	TypeMap::value_type("RESERVED4000PLUS", FieldTrait::fst_Reserved4000Plus)
};
const TypeMap FieldSpec::_typemap(FieldSpec::_Types, FieldSpec::_Types + sizeof(FieldSpec::_Types)/sizeof(TypeMap::value_type));

const CPPTypeMap::value_type FieldSpec::_CPPTypes[] =
{
	CPPTypeMap::value_type(FieldTrait::ft_int, "int"),
	CPPTypeMap::value_type(FieldTrait::ft_float, "double"),
	CPPTypeMap::value_type(FieldTrait::ft_char, "char"),
	CPPTypeMap::value_type(FieldTrait::ft_string, "std::string"),
	CPPTypeMap::value_type(FieldTrait::ft_pattern, "std::string")
};
const CPPTypeMap FieldSpec::_cpptypemap(FieldSpec::_CPPTypes, FieldSpec::_CPPTypes + sizeof(FieldSpec::_CPPTypes)/sizeof(CPPTypeMap::value_type));

const FIXCPPTypeMap::value_type FieldSpec::_FIXCPPTypes[] =
{
	FIXCPPTypeMap::value_type(FieldTrait::ft_int, "int"),
	FIXCPPTypeMap::value_type(FieldTrait::ft_float, "double"),
	FIXCPPTypeMap::value_type(FieldTrait::ft_char, "char"),
	FIXCPPTypeMap::value_type(FieldTrait::ft_string, "std::string"),
	FIXCPPTypeMap::value_type(FieldTrait::ft_pattern, "std::string")
};
const FIXCPPTypeMap FieldSpec::_fixcpptypemap
	(FieldSpec::_FIXCPPTypes, FieldSpec::_FIXCPPTypes + sizeof(FieldSpec::_FIXCPPTypes)/sizeof(FIXCPPTypeMap::value_type));
//-----------------------------------------------------------------------------------------

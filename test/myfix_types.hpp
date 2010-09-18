// *** f8c generated file: Do Not Edit
//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <www@orbweb.org>
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

#endif

//-------------------------------------------------------------------------------------------------
#ifndef _myfix_types_
#define _myfix_types_

namespace FIX8 {
namespace FIX441 {

//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 8> BeginString;
//-------------------------------------------------------------------------------------------------
typedef Field<Length, 9> BodyLength;
//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 10> CheckSum;
//-------------------------------------------------------------------------------------------------
typedef Field<SeqNum, 34> MsgSeqNum;
//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 35> MsgType;
const std::string MsgType_HEARTBEAT("0");
const std::string MsgType_TEST_REQUEST("1");
const std::string MsgType_RESEND_REQUEST("2");
const std::string MsgType_REJECT("3");
const std::string MsgType_SEQUENCE_RESET("4");
const std::string MsgType_LOGOUT("5");
const std::string MsgType_INDICATION_OF_INTEREST("6");
const std::string MsgType_ADVERTISEMENT("7");
const std::string MsgType_EXECUTION_REPORT("8");
const std::string MsgType_ORDER_CANCEL_REJECT("9");
const std::string MsgType_LOGON("A");
const std::string MsgType_DERIVATIVE_SECURITY_LIST("AA");
const std::string MsgType_NEW_ORDER_MULTILEG("AB");
const std::string MsgType_MULTILEG_ORDER_CANCEL_REPLACE("AC");
const std::string MsgType_TRADE_CAPTURE_REPORT_REQUEST("AD");
const std::string MsgType_TRADE_CAPTURE_REPORT("AE");
const std::string MsgType_ORDER_MASS_STATUS_REQUEST("AF");
const std::string MsgType_QUOTE_REQUEST_REJECT("AG");
const std::string MsgType_RFQ_REQUEST("AH");
const std::string MsgType_QUOTE_STATUS_REPORT("AI");
const std::string MsgType_QUOTE_RESPONSE("AJ");
const std::string MsgType_CONFIRMATION("AK");
const std::string MsgType_POSITION_MAINTENANCE_REQUEST("AL");
const std::string MsgType_POSITION_MAINTENANCE_REPORT("AM");
const std::string MsgType_REQUEST_FOR_POSITIONS("AN");
const std::string MsgType_REQUEST_FOR_POSITIONS_ACK("AO");
const std::string MsgType_POSITION_REPORT("AP");
const std::string MsgType_TRADE_CAPTURE_REPORT_REQUEST_ACK("AQ");
const std::string MsgType_TRADE_CAPTURE_REPORT_ACK("AR");
const std::string MsgType_ALLOCATION_REPORT("AS");
const std::string MsgType_ALLOCATION_REPORT_ACK("AT");
const std::string MsgType_CONFIRMATION_ACK("AU");
const std::string MsgType_SETTLEMENT_INSTRUCTION_REQUEST("AV");
const std::string MsgType_ASSIGNMENT_REPORT("AW");
const std::string MsgType_COLLATERAL_REQUEST("AX");
const std::string MsgType_COLLATERAL_ASSIGNMENT("AY");
const std::string MsgType_COLLATERAL_RESPONSE("AZ");
const std::string MsgType_NEWS("B");
const std::string MsgType_COLLATERAL_REPORT("BA");
const std::string MsgType_COLLATERAL_INQUIRY("BB");
const std::string MsgType_NETWORK_STATUS_REQUEST("BC");
const std::string MsgType_NETWORK_STATUS_RESPONSE("BD");
const std::string MsgType_USER_REQUEST("BE");
const std::string MsgType_USER_RESPONSE("BF");
const std::string MsgType_COLLATERAL_INQUIRY_ACK("BG");
const std::string MsgType_CONFIRMATION_REQUEST("BH");
const std::string MsgType_EMAIL("C");
const std::string MsgType_ORDER_SINGLE("D");
const std::string MsgType_ORDER_LIST("E");
const std::string MsgType_ORDER_CANCEL_REQUEST("F");
const std::string MsgType_ORDER_CANCEL_REPLACE_REQUEST("G");
const std::string MsgType_ORDER_STATUS_REQUEST("H");
const std::string MsgType_ALLOCATION_INSTRUCTION("J");
const std::string MsgType_LIST_CANCEL_REQUEST("K");
const std::string MsgType_LIST_EXECUTE("L");
const std::string MsgType_LIST_STATUS_REQUEST("M");
const std::string MsgType_LIST_STATUS("N");
const std::string MsgType_ALLOCATION_INSTRUCTION_ACK("P");
const std::string MsgType_DONT_KNOW_TRADE("Q");
const std::string MsgType_QUOTE_REQUEST("R");
const std::string MsgType_QUOTE("S");
const std::string MsgType_SETTLEMENT_INSTRUCTIONS("T");
const std::string MsgType_Undifferentiated("U");
const std::string MsgType_BaseContractDownloadRequest("U0");
const std::string MsgType_BaseContractDownload("U1");
const std::string MsgType_FilteredMarketUpdateRequest("U10");
const std::string MsgType_FilteredMarketDownloadRequest("U11");
const std::string MsgType_AtBestRequest("U12");
const std::string MsgType_AtBestUpdate("U13");
const std::string MsgType_DepthOfMarketRequest("U14");
const std::string MsgType_DepthOfMarketUpdate("U15");
const std::string MsgType_UserTextMessage("U16");
const std::string MsgType_RequestForQuoteMessage("U17");
const std::string MsgType_CustomMarketUpdate("U18");
const std::string MsgType_NewCustomOrder("U19");
const std::string MsgType_ContractDownloadRequest("U2");
const std::string MsgType_CustomOrderExecutionReport("U20");
const std::string MsgType_ContractDownload("U3");
const std::string MsgType_MarketStateDownloadRequest("U4");
const std::string MsgType_MarketStateDownload("U5");
const std::string MsgType_MarketUpdate("U6");
const std::string MsgType_TradeOrderDownloadRequest("U7");
const std::string MsgType_TradeOrderDownloadComplete("U8");
const std::string MsgType_UserDefinedRequestReject("U9");
const std::string MsgType_MARKET_DATA_REQUEST("V");
const std::string MsgType_MARKET_DATA_SNAPSHOT_FULL_REFRESH("W");
const std::string MsgType_MARKET_DATA_INCREMENTAL_REFRESH("X");
const std::string MsgType_MARKET_DATA_REQUEST_REJECT("Y");
const std::string MsgType_QUOTE_CANCEL("Z");
const std::string MsgType_QUOTE_STATUS_REQUEST("a");
const std::string MsgType_MASS_QUOTE_ACKNOWLEDGEMENT("b");
const std::string MsgType_SECURITY_DEFINITION_REQUEST("c");
const std::string MsgType_SECURITY_DEFINITION("d");
const std::string MsgType_SECURITY_STATUS_REQUEST("e");
const std::string MsgType_SECURITY_STATUS("f");
const std::string MsgType_TRADING_SESSION_STATUS_REQUEST("g");
const std::string MsgType_TRADING_SESSION_STATUS("h");
const std::string MsgType_MASS_QUOTE("i");
const std::string MsgType_BUSINESS_MESSAGE_REJECT("j");
const std::string MsgType_BID_REQUEST("k");
const std::string MsgType_BID_RESPONSE("l");
const std::string MsgType_LIST_STRIKE_PRICE("m");
const std::string MsgType_XML_MESSAGE("n");
const std::string MsgType_REGISTRATION_INSTRUCTIONS("o");
const std::string MsgType_REGISTRATION_INSTRUCTIONS_RESPONSE("p");
const std::string MsgType_ORDER_MASS_CANCEL_REQUEST("q");
const std::string MsgType_ORDER_MASS_CANCEL_REPORT("r");
const std::string MsgType_NEW_ORDER_CROSS("s");
const std::string MsgType_CROSS_ORDER_CANCEL_REPLACE_REQUEST("t");
const std::string MsgType_CROSS_ORDER_CANCEL_REQUEST("u");
const std::string MsgType_SECURITY_TYPE_REQUEST("v");
const std::string MsgType_SECURITY_TYPES("w");
const std::string MsgType_SECURITY_LIST_REQUEST("x");
const std::string MsgType_SECURITY_LIST("y");
const std::string MsgType_DERIVATIVE_SECURITY_LIST_REQUEST("z");
const size_t MsgType_dom_els(115);
//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 49> SenderCompID;
//-------------------------------------------------------------------------------------------------
typedef Field<UTCTimestamp, 52> SendingTime;
//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 56> TargetCompID;
//-------------------------------------------------------------------------------------------------
typedef Field<Length, 95> RawDataLength;
//-------------------------------------------------------------------------------------------------
typedef Field<data, 96> RawData;
//-------------------------------------------------------------------------------------------------
typedef Field<int, 98> EncryptMethod;
const int EncryptMethod_NONE_OTHER(0);
const int EncryptMethod_PKCS(1);
const int EncryptMethod_DES(2);
const int EncryptMethod_PKCS_DES(3);
const int EncryptMethod_PGP_DES(4);
const int EncryptMethod_PGP_DES_MD5(5);
const int EncryptMethod_PEM_DES_MD5(6);
const size_t EncryptMethod_dom_els(7);
//-------------------------------------------------------------------------------------------------
typedef Field<int, 108> HeartBtInt;
//-------------------------------------------------------------------------------------------------
typedef Field<std::string, 112> TestReqID;
//-------------------------------------------------------------------------------------------------
typedef Field<Boolean, 141> ResetSeqNumFlag;
//-------------------------------------------------------------------------------------------------
typedef GeneratedTable<unsigned, BaseEntry, 441> Myfix;

} // namespace FIX441
} // namespace FIX8
#endif // _myfix_types_

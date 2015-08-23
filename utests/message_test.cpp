//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

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
//-----------------------------------------------------------------------------------------
// f8 headers
#include "precomp.hpp"
#include <fix8/f8includes.hpp>
#include "gtest/gtest.h"
#include "utest_types.hpp"
#include "utest_router.hpp"
#include "utest_classes.hpp"

using namespace FIX8;
using namespace FIX8::UTEST;

/*!message unknown field
    \param message test suit name
    \param unknown_field test case name*/

TEST(message, unknown_field)
{
	// tag 1234 is undefined
    EXPECT_NO_THROW(Message::factory(ctx(),
		 "8=FIX.4.2\0019=72\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\0011234=blah\00198=0\00110=094\001", false, true));
    EXPECT_THROW(Message::factory(ctx(),
		 "8=FIX.4.2\0019=72\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\0011234=blah\00198=0\00110=094\001", false, false),
			 FIX8::f8Exception);
}

/*!message unknown field repeating group
    \param message test suit name
    \param unknown_field repeating group test case name*/
TEST(message, unknown_field_repeating_group)
{
	// tag 1234 is undefined
    EXPECT_NO_THROW(Message::factory(ctx(),
		"8=FIX.4.2\0019=173\00135=B\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001"
		"52=20131125-02:19:46.108\0011234=blah\001148=Here is the nws\00133=4\00158=The rain in Spain\001"
		"58=stays mainly\00158=on the plain\00158=End bulletin\00110=135\001", false, true));
    EXPECT_THROW(Message::factory(ctx(),
		"8=FIX.4.2\0019=173\00135=B\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001"
		"52=20131125-02:19:46.108\0011234=blah\001148=Here is the nws\00133=4\00158=The rain in Spain\001"
		"58=stays mainly\00158=on the plain\00158=End bulletin\00110=135\001", false, false), FIX8::f8Exception);
}

/*!message suppress chksum checking
    \param message test suit name
    \param suppress_chksum_checking test case name*/

TEST(message, suppress_chksum_checking)
{
	// chksum is wrong
    EXPECT_NO_THROW(Message::factory(ctx(),
		 "8=FIX.4.2\0019=62\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=094\001", true, false));
    EXPECT_THROW(Message::factory(ctx(),
		 "8=FIX.4.2\0019=62\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=094\001", false, false),
			 FIX8::f8Exception);
}

/*!message missing mandatory field
    \param message test suit name
    \param missing_field test case name*/

TEST(message, missing_field)
{
    EXPECT_THROW(Message::factory(ctx(),
		 "8=FIX.4.2\0019=12\001\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=098\001"), std::exception);
}

/*!logon decoding test
    \param message test suit name
    \param logon_decode test case name*/

TEST(message, logon_decode)
{
    Message * logon = Message::factory(ctx(),
		 "8=FIX.4.2\0019=12\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=185\001");

    EXPECT_EQ("A", logon->get_msgtype());
    EXPECT_TRUE(logon->is_admin());
    EXPECT_TRUE(logon->Header() != 0);
    EXPECT_TRUE(logon->Trailer() != 0);

    delete logon;
    logon = 0;
}

#define FIELD_TEST(field, expect, message, test_fun) {\
    UTEST::field value; \
    message->get(value); \
    test_fun(expect, value()); \
}

/*!new order entry decoding test
    \param message test suit name
    \param neworder_decode test case name*/

TEST(message, neworder_decode)
{
    Message * neworder = Message::factory(ctx(),
		 "8=FIX.4.2\0019=220\00135=D\00149=CLIENT\00156=SERVER\00134=78\00150=S\001"
		 "142=US,IL\00157=G\00152=20130304-05:06:14\00111=4\0011=54129\00121=1\00155=OC\001"
		 "167=OPT\001107=TEST SYMBOL\00154=1\001449=20130304\001450=05:06:14\001743=201303\001"
		 "60=20130304-05:06:14\00138=50.00\00140=2\00144=400.50\00159=0\00158=TEST\00110=051\001");

    EXPECT_EQ("D", neworder->get_msgtype());
    EXPECT_FALSE(neworder->is_admin());
    EXPECT_TRUE(neworder->Header() != 0);
    EXPECT_TRUE(neworder->Trailer() != 0);

    MessageBase * header = neworder->Header();

    EXPECT_TRUE(header->has<BeginString>());
    EXPECT_TRUE(header->has<SendingTime>());
    EXPECT_TRUE(header->has<MsgSeqNum>());
    EXPECT_TRUE(header->has<SenderCompID>());
    EXPECT_TRUE(header->has<TargetCompID>());
    EXPECT_TRUE(header->has<SenderSubID>());
    EXPECT_TRUE(header->has<SenderLocationID>());
    EXPECT_TRUE(header->has<TargetSubID>());

    EXPECT_TRUE(neworder->has<ClOrdID>());
    EXPECT_TRUE(neworder->has<Account>());
    EXPECT_TRUE(neworder->has<Symbol>());
    EXPECT_TRUE(neworder->has<SecurityType>());
    EXPECT_TRUE(neworder->has<SecurityDesc>());
    EXPECT_TRUE(neworder->has<Side>());
    EXPECT_TRUE(neworder->has<OrderQty>());
    EXPECT_TRUE(neworder->has<OrdType>());
    EXPECT_TRUE(neworder->has<Price>());
    EXPECT_TRUE(neworder->has<TimeInForce>());

    EXPECT_TRUE(neworder->has<TotalVolumeTradedDate>());
    EXPECT_TRUE(neworder->has<TotalVolumeTradedTime>());
    EXPECT_TRUE(neworder->has<DeliveryDate>());

    FIELD_TEST(BeginString, "FIX.4.2", header, EXPECT_EQ);
    //FIELD_TEST(SendingTime, Poco::DateTime(2013, 3, 4, 5, 6, 14), header, EXPECT_EQ);
    tm tms { 14, 6, 5, 4, 2, 113 }; // mon is 0-11, year is from 1900
    SendingTime st(tms);
    FIELD_TEST(SendingTime, st.get(), header, EXPECT_EQ);

    FIELD_TEST(MsgSeqNum, 78, header, EXPECT_EQ);
    FIELD_TEST(SenderCompID, "CLIENT", header, EXPECT_EQ);
    FIELD_TEST(TargetCompID , "SERVER", header, EXPECT_EQ);
    FIELD_TEST(SenderSubID, "S", header, EXPECT_EQ);
    FIELD_TEST(SenderLocationID, "US,IL", header, EXPECT_EQ);
    FIELD_TEST(TargetSubID, "G", header, EXPECT_EQ);
    FIELD_TEST(TargetSubID, "G", header, EXPECT_EQ);

    FIELD_TEST(ClOrdID, "4", neworder, EXPECT_EQ);
    FIELD_TEST(Account, "54129", neworder, EXPECT_EQ);
    FIELD_TEST(HandlInst, '1', neworder, EXPECT_EQ);
    FIELD_TEST(SecurityDesc, "TEST SYMBOL", neworder, EXPECT_EQ);
    FIELD_TEST(Symbol, "OC", neworder, EXPECT_EQ);
    FIELD_TEST(SecurityType, "OPT", neworder, EXPECT_EQ);
    FIELD_TEST(ClOrdID, "4", neworder, EXPECT_EQ);
    FIELD_TEST(Side, '1', neworder, EXPECT_EQ);
    FIELD_TEST(OrderQty, 50, neworder, EXPECT_EQ);
    FIELD_TEST(OrdType, '2', neworder, EXPECT_EQ);
    FIELD_TEST(Price, 400.5, neworder, EXPECT_EQ);
    FIELD_TEST(TimeInForce, '0', neworder, EXPECT_EQ);
    FIELD_TEST(Text, "TEST", neworder, EXPECT_EQ);

	 // test UTCDateOnly
    tm tvtds { 0, 0, 0, 4, 2, 113 };
    TotalVolumeTradedDate tvtd(tvtds);
    FIELD_TEST(TotalVolumeTradedDate, tvtd.get(), neworder, EXPECT_EQ);
	 // test UTCTimeOnly
    tm tvtts { 14, 6, 5 };
    TotalVolumeTradedTime tvtt(tvtts);
    FIELD_TEST(TotalVolumeTradedTime, tvtt.get(), neworder, EXPECT_EQ);
	 // test MonthYear
    tm tvdds { 0, 0, 0, 0, 2, 113 };
    DeliveryDate tvdd(tvdds);
    FIELD_TEST(DeliveryDate, tvdd.get(), neworder, EXPECT_EQ);

    delete neworder;
    neworder = 0;
}


/*!new order entry (with custom field) decoding test
    \param message test suit name
    \param neworder_custom_decode test case name*/

TEST(message, neworder_custom_decode)
{
    Message * neworder = Message::factory(ctx(),
		 "8=FIX.4.2\0019=194\00135=D\00149=CLIENT\00156=SERVER\00134=78\00150=S\001142=US,IL\00157=G\001"
		 "52=20130304-05:06:14\00111=4\0011=54129\00121=1\00155=OC\001167=OPT\001107=TEST SYMBOL\00154=1\001"
		 "60=20130304-05:06:14\00138=50.00\00140=2\00144=400.50\00159=0\00158=TEST\0019999=HELLO\00110=231\001");

    EXPECT_TRUE(neworder->has<SampleUserField>());
    delete neworder;
    neworder = 0;
}

/*!new order entry (with custom field) encoding test
    \param message test suit name
    \param neworder_custom_encode test case name*/
TEST(message, neworder_custom_encode)
{
    NewOrderSingle *nos(new NewOrderSingle);
    *nos->Header() << new msg_seq_num(78)
                   << new sender_comp_id("A12345B")
                   << new sending_time("20130305-02:19:46.108")
                   << new target_comp_id("COMPARO");

    *nos << new TransactTime("20130305-02:19:46.108")
         << new Account("01234567")
         << new OrderQty(50)
         << new Price(400.5)
         << new ClOrdID("4")
         << new HandlInst(HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
         << new OrdType(OrdType_LIMIT)
         << new Side(Side_BUY)
         << new Symbol("OC")
         << new TimeInForce(TimeInForce_DAY)
			<< new SampleUserField("HELLO");

    f8String output;
    nos->encode(output);

    f8String expect(
		"8=FIX.4.2\0019=153\00135=D\00149=A12345B\00156=COMPARO\00134=78\001"
		"52=20130305-02:19:46.108\0019999=HELLO\00111=4\0011=01234567\00121=1\001"
		"55=OC\00154=1\00160=20130305-02:19:46.108\00138=50.0\00140=2\00144=400.5\00159=0\00110=199\001");

    EXPECT_EQ(expect, output);

    delete nos;
    nos = 0;
}

/*!new order entry (with repeating groups) decoding test
    \param message test suit name
    \param neworder_group_decode test case name*/

TEST(message, neworder_group_decode)
{
    Message * neworder = Message::factory(ctx(),
		 "8=FIX.4.2\0019=190\00135=D\00149=CLIENT\00156=SERVER\00134=78\00150=S\001142=US,IL\00157=G\001"
		 "52=20130304-05:06:14\00111=4\0011=54129\00121=1\00155=OC\001167=OPT\001107=TEST SYMBOL\00154=1\001"
		 "60=20130304-05:06:14\00138=50.00\00140=2\00144=400.50\00159=0\00158=TEST\00178=2\00179=FIRST\001"
		 "80=20\00179=SECOND\00180=30\00110=221\001");

    const GroupBase *grnoul(neworder->find_group<NewOrderSingle::NoAllocs>());
    EXPECT_TRUE(grnoul != 0);
    EXPECT_EQ(size_t(2), grnoul->size());

    FIELD_TEST(AllocAccount, "FIRST", grnoul->get_element(0), EXPECT_EQ);
    FIELD_TEST(AllocShares, 20, grnoul->get_element(0), EXPECT_EQ);

    FIELD_TEST(AllocAccount, "SECOND", grnoul->get_element(1), EXPECT_EQ);
    FIELD_TEST(AllocShares, 30, grnoul->get_element(1), EXPECT_EQ);

    delete neworder;
    neworder = 0;
}

/*!checksum test
    \param message test suit name
    \param calc_chksum test case name*/

TEST(message, calc_chksum)
{
    f8String msg(
		 "8=FIX.4.2\0019=117\00135=5\00134=3725\001369=617\00152=20130304-07:25:37.403\001"
		 "49=CME\00150=G\00156=1G9125N\00157=ADMIN\001143=US,IL\00158=Logout confirmed.\001789=618\00110=121\001");

    EXPECT_EQ(unsigned(172), Message::calc_chksum(msg));
    EXPECT_EQ(unsigned(121), Message::calc_chksum(msg, 0, static_cast<int>(msg.length()-7)));
    EXPECT_EQ(unsigned(15), Message::calc_chksum(msg, 10, 5));
    EXPECT_EQ(unsigned(16), Message::calc_chksum(msg, 10, 6));

    f8String empty;
    EXPECT_EQ(unsigned(0),Message::calc_chksum(empty));

    EXPECT_EQ(unsigned(172), Message::calc_chksum(msg.c_str(), static_cast<int>(msg.length())));
    EXPECT_EQ(unsigned(121), Message::calc_chksum(msg.c_str(), static_cast<unsigned>(msg.length()), 0, static_cast<int>(msg.length())-7));
    EXPECT_EQ(unsigned(15), Message::calc_chksum(msg.c_str(), static_cast<unsigned>(msg.length()), 10, 5));
    EXPECT_EQ(unsigned(16), Message::calc_chksum(msg.c_str(), static_cast<unsigned>(msg.length()), 10, 6));
    EXPECT_EQ(unsigned(0), Message::calc_chksum(empty.c_str()));
}

/*!checksum formating test
    \param message test suit name
    \param fmt_chksum test case name*/

TEST(message, fmt_chksum)
{
    EXPECT_EQ("000", Message::fmt_chksum(0));
    EXPECT_EQ("001", Message::fmt_chksum(1));
    EXPECT_EQ("023", Message::fmt_chksum(23));
    EXPECT_EQ("999", Message::fmt_chksum(999));
}

/*!helper to test MessageBase::extract_element
    \param msg field string
    \param expect_tag expected field tag
    \param expect_val expected field val*/

void extract_element_test(f8String msg, f8String expect_tag, f8String expect_val)
{
    char cVal[FIX8_MAX_FLD_LENGTH];
    char cTag[FIX8_MAX_FLD_LENGTH];
    MessageBase::extract_element(msg.c_str(), static_cast<unsigned>(msg.length()), cTag, cVal);
    EXPECT_EQ(expect_val, f8String(cVal));
    EXPECT_EQ(expect_tag, f8String(cTag));

    f8String sVal;
    f8String sTag;
    MessageBase::extract_element(msg.c_str(), static_cast<unsigned>(msg.length()), sTag, sVal);
    EXPECT_EQ(expect_val, sVal);
    EXPECT_EQ(expect_tag, sTag);
}

/*!MessageBase::extract_element test
    \param message test suit name
    \param extract_element test case name*/

TEST(message, extract_element)
{
    extract_element_test("8=FIX.4.2", "8", "FIX.4.2");
    extract_element_test("8=", "8", "");
    extract_element_test("=FIX.4.2", "", "FIX.4.2");
    extract_element_test("", "", "");
}

/*!logon encoding test
    \param message test suit name
    \param logon_encode test case name*/

TEST(message, logon_encode)
{
    Logon * logon(new Logon);
    f8String output;
    logon->encode(output);

    f8String expect("8=FIX.4.2\0019=5\00135=A\00110=178\001");
    EXPECT_EQ(expect, output);

    delete logon;
    logon = 0;
}

/*!new order entry encoding test
    \param message test suit name
    \param neworder_encode test case name*/

TEST(message, neworder_encode)
{
    NewOrderSingle *nos(new NewOrderSingle);
    *nos->Header() << new msg_seq_num(78)
                   << new sender_comp_id("A12345B")
                   << new SenderSubID("2DEFGH4")
                   << new sending_time("20130305-02:19:46.108")
                   << new target_comp_id("COMPARO")
                   << new TargetSubID("G")
                   << new SenderLocationID("AU,SY");

    *nos << new TransactTime("20130305-02:19:46.108")
         << new Account("01234567")
         << new OrderQty(50)
         << new Price(400.5)
         << new ClOrdID("4")
         << new HandlInst(HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
         << new OrdType(OrdType_LIMIT)
         << new Side(Side_BUY)
         << new Symbol("OC")
         << new Text("NIGEL")
         << new TimeInForce(TimeInForce_DAY)
         << new SecurityDesc("AOZ3 C02000")
         << new SecurityType(SecurityType_OPTION)
			<< new TotalVolumeTradedDate("20130305")
			<< new TotalVolumeTradedTime("02:19:46.123")
			<< new DeliveryDate("201303");

    f8String output;
    nos->encode(output);

    f8String expect(
		 "8=FIX.4.2\0019=244\00135=D\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001"
		 "142=AU,SY\00157=G\00152=20130305-02:19:46.108\00111=4\0011=01234567\00121=1\001"
		 "449=20130305\001450=02:19:46.123\001743=20130301\00155=OC\001"
		 "167=OPT\001107=AOZ3 C02000\00154=1\00160=20130305-02:19:46.108\00138=50.0\00140=2\001"
		 "44=400.5\00159=0\00158=NIGEL\00110=022\001");

    EXPECT_EQ(expect, output);

    delete nos;
    nos = 0;
}

/*!news message (with repeating groups) encoding test
    \param message test suit name
    \param repeating_group test case name*/

TEST(message, repeating_group)
{
    News *nws(new News);
    *nws->Header() << new msg_seq_num(78)
                   << new sender_comp_id("A12345B")
                   << new SenderSubID("2DEFGH4")
                   << new sending_time("20131125-02:19:46.108")
                   << new target_comp_id("COMPARO");

    *nws << new Headline("Here is the nws");
    *nws << new LinesOfText(4);
    GroupBase *lines(nws->find_group<News::LinesOfText>());

    MessageBase *gr1(lines->create_group());
    *gr1 << new Text("The rain in Spain");
    *lines << gr1;
    MessageBase *gr2(lines->create_group());
    *gr2 << new Text("stays mainly");
    *lines << gr2;
    MessageBase *gr3(lines->create_group());
    *gr3 << new Text("on the plain");
    *lines << gr3;
    MessageBase *gr4(lines->create_group());
    *gr4 << new Text("End bulletin");
    *lines << gr4;
	 *nws << lines;

    f8String output;
    nws->encode(output);

    f8String expect("8=FIX.4.2\0019=163\00135=B\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001"
		 "52=20131125-02:19:46.108\001148=Here is the nws\00133=4\00158=The rain in Spain\001"
		 "58=stays mainly\00158=on the plain\00158=End bulletin\00110=231\001");

    EXPECT_EQ(expect, output);

    delete nws;
    nws = 0;
}

/*!new order entry (with repeating groups) encoding test
    \param message test suit name
    \param nestedGroup_encode test case name*/

TEST(message, nestedGroup_encode)
{
    NewOrderList * nol(new NewOrderList);
    *nol->Header() << new msg_seq_num(78)
                   << new sender_comp_id("A12345B")
                   << new SenderSubID("2DEFGH4")
                   << new sending_time("20130305-02:19:46.108")
                   << new target_comp_id("COMPARO")
                   << new TargetSubID("G")
                   << new SenderLocationID("AU,SY");

    *nol << new ListID("123")
         << new BidType(1);

    *nol << new TotNoOrders(2);

    *nol << new NoOrders(2);
    GroupBase *noul(nol->find_group<NewOrderList::NoOrders>());

    MessageBase * gr1(noul->create_group());
    *gr1 << new ClOrdID("1") << new ListSeqNo("1") << new Account("TEST");
    *noul << gr1;

    MessageBase * gr2(noul->create_group());
    *gr2 << new ClOrdID("2") << new ListSeqNo("2");

    *gr2 << new NoAllocs(2);
    GroupBase *nested(gr2->find_group<NewOrderList::NoOrders::NoAllocs>());

    MessageBase * ngr1(nested->create_group());
    *ngr1 << new AllocAccount("first") << new AllocShares(10);
    *nested << ngr1;

    MessageBase * ngr2(nested->create_group());
    *ngr2 << new AllocAccount("second") << new AllocShares(20);
    *nested << ngr2;

    *noul << gr2;

    f8String output;
    nol->encode(output);

    f8String expect(
		 "8=FIX.4.2\0019=174\00135=E\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001"
		 "142=AU,SY\00157=G\00152=20130305-02:19:46.108\00166=123\001394=1\00168=2\00173=2\001"
		 "11=1\00167=1\0011=TEST\00111=2\00167=2\00178=2\00179=first\00180=10.0\001"
		 "79=second\00180=20.0\00110=064\001");

    EXPECT_EQ(expect, output);

    delete nol;
    nol = 0;
}


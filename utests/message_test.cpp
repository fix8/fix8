// f8 headers
#include <f8headers.hpp>
#include <f8includes.hpp>
#include "gtest/gtest.h"
#include "utest_types.hpp"
#include "utest_router.hpp"
#include "utest_classes.hpp"

using namespace FIX8;
using namespace FIX8::UTEST;

TEST(message, missing_field)
{
    EXPECT_THROW(Message::factory(ctx, "8=FIX.4.2\0019=12\001\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=098\001"), std::exception);
}

TEST(message, logon_decode)
{
    Message * logon = Message::factory(ctx, "8=FIX.4.2\0019=12\00135=A\00134=1\00149=CLIENT\00156=SERVER\00152=20130304-02:44:30\001108=30\00198=0\00110=185\001");

    EXPECT_EQ("A", logon->get_msgtype());
    EXPECT_TRUE(logon->is_admin());
    EXPECT_TRUE(logon->Header() != NULL);
    EXPECT_TRUE(logon->Trailer() != NULL);

    delete logon;
    logon = NULL;
}

#define FIELD_TEST(field, expect, message, test_fun) {\
    UTEST::field value; \
    message->get(value); \
    test_fun(expect, value()); \
}

TEST(message, neworder_decode)
{
    Message * neworder = Message::factory(ctx, "8=FIX.4.2\0019=190\00135=D\00149=CLIENT\00156=SERVER\00134=78\00150=S\001142=US,IL\00157=G\00152=20130304-05:06:14\00111=4\0011=54129\00121=1\00155=OC\001167=OPT\001107=TEST SYMBOL\00154=1\00160=20130304-05:06:14\00138=50.00\00140=2\00144=400.50\00159=0\00158=TEST\00110=077\001");

    EXPECT_EQ("D", neworder->get_msgtype());
    EXPECT_FALSE(neworder->is_admin());
    EXPECT_TRUE(neworder->Header() != NULL);
    EXPECT_TRUE(neworder->Trailer() != NULL);

    MessageBase * header = neworder->Header();

    EXPECT_TRUE(header->has<UTEST::BeginString>());
    EXPECT_TRUE(header->has<UTEST::SendingTime>());
    EXPECT_TRUE(header->has<UTEST::MsgSeqNum>());
    EXPECT_TRUE(header->has<UTEST::SenderCompID>());
    EXPECT_TRUE(header->has<UTEST::TargetCompID>());
    EXPECT_TRUE(header->has<UTEST::SenderSubID>());
    EXPECT_TRUE(header->has<UTEST::SenderLocationID>());
    EXPECT_TRUE(header->has<UTEST::TargetSubID>());

    EXPECT_TRUE(neworder->has<UTEST::ClOrdID>());
    EXPECT_TRUE(neworder->has<UTEST::Account>());
    EXPECT_TRUE(neworder->has<UTEST::Symbol>());
    EXPECT_TRUE(neworder->has<UTEST::SecurityType>());
    EXPECT_TRUE(neworder->has<UTEST::SecurityDesc>());
    EXPECT_TRUE(neworder->has<UTEST::Side>());
    EXPECT_TRUE(neworder->has<UTEST::OrderQty>());
    EXPECT_TRUE(neworder->has<UTEST::OrdType>());
    EXPECT_TRUE(neworder->has<UTEST::Price>());
    EXPECT_TRUE(neworder->has<UTEST::TimeInForce>());

    FIELD_TEST(BeginString, "FIX.4.2", header, EXPECT_EQ);
    FIELD_TEST(SendingTime, Poco::DateTime(2013, 3, 4, 5, 6, 14), header, EXPECT_EQ);
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

    delete neworder;
    neworder = NULL;
}

TEST(message, neworder_group_decode)
{
    Message * neworder = Message::factory(ctx, "8=FIX.4.2\0019=190\00135=D\00149=CLIENT\00156=SERVER\00134=78\00150=S\001142=US,IL\00157=G\00152=20130304-05:06:14\00111=4\0011=54129\00121=1\00155=OC\001167=OPT\001107=TEST SYMBOL\00154=1\00160=20130304-05:06:14\00138=50.00\00140=2\00144=400.50\00159=0\00158=TEST\00178=2\00179=FIRST\00180=20\00179=SECOND\00180=30\00110=221\001");

    const GroupBase *grnoul(neworder->find_group<NewOrderSingle::NoAllocs>());
    EXPECT_TRUE(grnoul != NULL);
    EXPECT_EQ(size_t(2), grnoul->size());

    FIELD_TEST(AllocAccount, "FIRST", grnoul->get_element(0), EXPECT_EQ);
    FIELD_TEST(AllocShares, 20, grnoul->get_element(0), EXPECT_EQ);

    FIELD_TEST(AllocAccount, "SECOND", grnoul->get_element(1), EXPECT_EQ);
    FIELD_TEST(AllocShares, 30, grnoul->get_element(1), EXPECT_EQ);

    delete neworder;
    neworder = NULL;
}


TEST(message, calc_chksum)
{
    f8String msg("8=FIX.4.2\0019=117\00135=5\00134=3725\001369=617\00152=20130304-07:25:37.403\00149=CME\00150=G\00156=1G9125N\00157=ADMIN\001143=US,IL\00158=Logout confirmed.\001789=618\00110=121\001");

    EXPECT_EQ(unsigned(172), Message::calc_chksum(msg));
    EXPECT_EQ(unsigned(121), Message::calc_chksum(msg, 0, msg.length()-7));
    EXPECT_EQ(unsigned(15), Message::calc_chksum(msg, 10, 5));
    EXPECT_EQ(unsigned(16), Message::calc_chksum(msg, 10, 6));

    f8String empty;
    EXPECT_EQ(unsigned(0),Message::calc_chksum(empty));

    EXPECT_EQ(unsigned(172), Message::calc_chksum(msg.c_str(), msg.length()));
    EXPECT_EQ(unsigned(121), Message::calc_chksum(msg.c_str(), msg.length(), 0, msg.length()-7));
    EXPECT_EQ(unsigned(15), Message::calc_chksum(msg.c_str(), msg.length(), 10, 5));
    EXPECT_EQ(unsigned(16), Message::calc_chksum(msg.c_str(), msg.length(), 10, 6));
    EXPECT_EQ(unsigned(0), Message::calc_chksum(empty.c_str()));
}

TEST(message, fmt_chksum)
{
    EXPECT_EQ("000", Message::fmt_chksum(0));
    EXPECT_EQ("001", Message::fmt_chksum(1));
    EXPECT_EQ("023", Message::fmt_chksum(23));
    EXPECT_EQ("999", Message::fmt_chksum(999));
}

void extract_element_test(f8String msg, f8String expect_tag, f8String expect_val)
{
    char cVal[MAX_FLD_LENGTH];
    char cTag[MAX_FLD_LENGTH];
    MessageBase::extract_element(msg.c_str(), msg.length(), cTag, cVal);
    EXPECT_EQ(expect_val, f8String(cVal));
    EXPECT_EQ(expect_tag, f8String(cTag));

    f8String sVal;
    f8String sTag;
    MessageBase::extract_element(msg.c_str(), msg.length(), sTag, sVal);
    EXPECT_EQ(expect_val, sVal);
    EXPECT_EQ(expect_tag, sTag);
}

TEST(message, extract_element)
{
    extract_element_test("8=FIX.4.2", "8", "FIX.4.2");
    extract_element_test("8=", "8", "");
    extract_element_test("=FIX.4.2", "", "FIX.4.2");
    extract_element_test("", "", "");
}

TEST(message, logon_encode)
{
    Logon * logon(new Logon);
    f8String output;
    logon->encode(output);

    f8String expect("8=FIX.4.2\0019=5\00135=A\00110=178\001");
    EXPECT_EQ(expect, output);

    delete logon;
    logon = NULL;
}

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
         << new SecurityType(SecurityType_OPTION);

    f8String output;
    nos->encode(output);

    f8String expect("8=FIX.4.2\0019=203\00135=D\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001142=AU,SY\00157=G\00152=20130305-02:19:46.108\00111=4\0011=01234567\00121=1\00155=OC\001167=OPT\001107=AOZ3 C02000\00154=1\00160=20130305-02:19:46.108\00138=50.00\00140=2\00144=400.50\00159=0\00158=NIGEL\00110=089\001");

    EXPECT_EQ(expect, output);

    delete nos;
    nos = NULL;
}

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

    f8String expect("8=FIX.4.2\0019=176\00135=E\00149=A12345B\00156=COMPARO\00134=78\00150=2DEFGH4\001142=AU,SY\00157=G\00152=20130305-02:19:46.108\00166=123\001394=1\00168=2\00173=2\00111=1\00167=1\0011=TEST\00111=2\00167=2\00178=2\00179=first\00180=10.00\00179=second\00180=20.00\00110=162\001");

    EXPECT_EQ(expect, output);

    delete nol;
    nol = NULL;

}



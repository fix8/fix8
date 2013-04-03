// f8 headers
#include <f8headers.hpp>
#include <f8config.h>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#ifdef PROFILING_BUILD
#include <sys/gmon.h>
#endif

#include <errno.h>
#include <f8exception.hpp>
#include <hypersleep.hpp>
#include <mpmc.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <xml.hpp>
#include <thread.hpp>
#include <gzstream.hpp>
#include <tickval.hpp>
#include <logger.hpp>
#include <traits.hpp>
#include <timer.hpp>
#include <field.hpp>
#include <message.hpp>
#include <mockConnection.hpp>
#include <session.hpp>
#include <configuration.hpp>
#include <persist.hpp>
#include <sessionwrapper.hpp>

#include "utest_types.hpp"
#include "utest_router.hpp"
#include "utest_classes.hpp"
#include "gtest/gtest.h"

//***************************************************************
//make sure the session.cpp will use the functions
//from mockConnection
#include "../runtime/session.cpp"
//***************************************************************

using namespace FIX8;
using namespace FIX8::UTEST;

TEST(sessionId, sessionId)
{
    //only well formated string supported
    SessionID id("FIX.4.2:sender->target");

    EXPECT_EQ(f8String("FIX.4.2"), id.get_beginString()());
    EXPECT_EQ(f8String("sender"), id.get_senderCompID()());
    EXPECT_EQ(f8String("target"), id.get_targetCompID()());

    SessionID empty_id(":->");
    EXPECT_TRUE(empty_id.get_beginString()().empty());
    EXPECT_TRUE(empty_id.get_senderCompID()().empty());
    EXPECT_TRUE(empty_id.get_targetCompID()().empty());

    SessionID error_id("");
    EXPECT_TRUE(error_id.get_beginString()().empty());
    EXPECT_TRUE(error_id.get_senderCompID()().empty());
    EXPECT_TRUE(error_id.get_targetCompID()().empty());

    SessionID part_id("FIX.4.2:->target");
    EXPECT_TRUE(part_id.get_beginString()().empty());
    EXPECT_TRUE(part_id.get_senderCompID()().empty());
    EXPECT_TRUE(part_id.get_targetCompID()().empty());
}

class test_session : public FIX8::Session
{
    utest_Router  _router;

public:
    test_session(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0,
        FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, sid, persist, logger, plogger)
    {
        _timer.clear();
        _timer.schedule(_hb_processor, 0);
        _timer.join();
    }

    ~test_session(){}

    bool handle_application(const unsigned seqnum, const FIX8::Message *msg)
    {
        return enforce(seqnum, msg) || msg->process(_router);
    }

    //helpers
    States::SessionStates getState() {return _state;}
    unsigned get_next_receive_seq() {return _next_receive_seq;}
    void set_last_received(Tickval& val) {_last_received = val;}
    void kickHBService() {heartbeat_service();}
};

class session_fixture
{
public:
    MemoryPersister * per;
    PipeLogger * pLogger;
    PipeLogger * sLogger;
    test_session * ss;

    session_fixture()
    {
        SessionID id("FIX.4.2:A12345B->COMPARO");

        f8String cmd("|/bin/cat");
        ebitset<Logger::Flags> flag;
        flag.set(Logger::timestamp, true);
        flag.set(Logger::sequence, true);
        flag.set(Logger::thread, true);
        flag.set(Logger::direction, true);
        flag.set(Logger::append, true);
        flag.set(Logger::pipe, true);

        per = new MemoryPersister;
        sLogger = new PipeLogger(cmd, flag);
        pLogger = new PipeLogger(cmd, flag);

        ss = new test_session(ctx, id, per, sLogger, pLogger);
    };

    ~session_fixture()
    {
        delete ss;
        delete sLogger;
        delete pLogger;
        delete per;
    };
};

class initiator_fixture : public session_fixture
{
public:
    ClientConnection * initiator;

    initiator_fixture()
    {
        Poco::Net::SocketAddress addr("127.0.0.1:80");
        initiator = new ClientConnection(NULL, addr, *ss, false, false);
        initiator->connect();
        ss->start(initiator, false);
    };

    ~initiator_fixture()
    {
        ss->stop();
        delete initiator;
    };
};

class sessionTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        recv_seq = 1;
    }

    static void TearDownTestCase()
    {
    }
public:
    static initiator_fixture initiator_test;
    static unsigned recv_seq;
};

initiator_fixture sessionTest::initiator_test;
unsigned sessionTest::recv_seq;

void fillSendHeader(MessageBase * header, unsigned seq)
{
    *header << new msg_seq_num(seq)
            << new sender_comp_id("A12345B")
            << new SenderSubID("2DEFGH4")
            << new sending_time()
            << new target_comp_id("COMPARO")
            << new TargetSubID("G")
            << new SenderLocationID("AU,SY");
}

void fillRecvHeader(MessageBase * header)
{
    *header << new msg_seq_num(sessionTest::recv_seq++)
            << new sender_comp_id("COMPARO")
            << new SenderSubID("G")
            << new sending_time()
            << new target_comp_id("A12345B")
            << new TargetSubID("2DEFGH4")
            << new SenderLocationID("AU,SY");
}

f8String getSingleMsg()
{
    if(sessionTest::initiator_test.initiator->_output.empty())
    {
        return "";
    }

    f8String output = sessionTest::initiator_test.initiator->_output[0];
    sessionTest::initiator_test.initiator->_output.clear();
    return output;
}

std::vector<f8String> getMsgs()
{
    std::vector<f8String> ret = sessionTest::initiator_test.initiator->_output;
    sessionTest::initiator_test.initiator->_output.clear();
    return ret;
}

void clearOutputs()
{
    sessionTest::initiator_test.initiator->_output.clear();
}

f8String composeOrder(unsigned seq)
{
    f8String tmp;
    NewOrderSingle *nos(new NewOrderSingle);
    fillSendHeader(nos->Header(),seq);
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

    nos->encode(tmp);
    delete nos;
    return tmp;
}

TEST_F(sessionTest, logon)
{
    //initiator send logon
    f8String output = getSingleMsg();
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=A") !=  std::string::npos);
    EXPECT_EQ(States::st_logon_sent, initiator_test.ss->getState());

    //receive logon confirm
    Logon * logon = new Logon;
    fillRecvHeader(logon->Header());

    *logon << new HeartBtInt(5) << new EncryptMethod(0);
    f8String logon_str;
    logon->encode(logon_str);
    initiator_test.ss->update_received();
    initiator_test.ss->process(logon_str);

    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());
    EXPECT_EQ(unsigned(5), initiator_test.initiator->get_hb_interval());

    delete logon;

    clearOutputs();
}

TEST_F(sessionTest, handle_resend_request)
{
    //no mess.ge available
    ResendRequest * resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(4);
    f8String resend_str;
    resend->encode(resend_str);
    delete resend;

    initiator_test.ss->set_persister(NULL);
    initiator_test.ss->update_received();
    initiator_test.ss->process(resend_str);

    f8String output = getSingleMsg();

    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());
    EXPECT_TRUE(output.find("35=4") !=  std::string::npos);

    //ill resend request
    initiator_test.ss->set_persister(initiator_test.per);
    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(100) << new EndSeqNo(4);
    resend->encode(resend_str);
    delete resend;

    initiator_test.ss->update_received();
    initiator_test.ss->process(resend_str);

    output = getSingleMsg();

    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());
    EXPECT_TRUE(output.find("35=3") !=  std::string::npos);

    //have records
    unsigned next_send = initiator_test.ss->get_next_send_seq();

    Reject * rej = new Reject;
    fillSendHeader(rej->Header(), next_send);
    *rej << new RefSeqNum(100);
    f8String first;
    rej->encode(first);
    delete rej;
    initiator_test.per->put(next_send, first);
    initiator_test.per->put(++next_send, recv_seq);

    f8String second = composeOrder(next_send);
    initiator_test.per->put(next_send, second);
    initiator_test.per->put(++next_send, recv_seq);

    f8String third = composeOrder(next_send);
    initiator_test.per->put(next_send, third);
    initiator_test.per->put(++next_send, recv_seq);

    f8String four = composeOrder(next_send);
    initiator_test.per->put(next_send, four);
    initiator_test.per->put(++next_send, recv_seq);

    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(4);
    resend->encode(resend_str);
    delete resend;

    initiator_test.ss->update_received();
    initiator_test.ss->process(resend_str);

    std::vector<f8String> outputs = getMsgs();

    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());
    EXPECT_EQ(first, outputs[0]);
    EXPECT_EQ(second, outputs[1]);
    EXPECT_EQ(third, outputs[2]);
    EXPECT_EQ(four, outputs[3]);

    //have seq gap
    ++next_send; //skip 5
    f8String six = composeOrder(next_send);
    initiator_test.per->put(next_send, six);
    initiator_test.per->put(++next_send, recv_seq);

    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(6);
    resend->encode(resend_str);
    delete resend;

    initiator_test.ss->update_received();
    initiator_test.ss->process(resend_str);

    outputs = getMsgs();

    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());
    EXPECT_EQ(first, outputs[0]);
    EXPECT_EQ(second, outputs[1]);
    EXPECT_EQ(third, outputs[2]);
    EXPECT_EQ(four, outputs[3]);
    EXPECT_TRUE(outputs[4].find("35=4") !=  std::string::npos);
    EXPECT_EQ(six, outputs[5]);

    clearOutputs();
}

TEST_F(sessionTest, handle_seq_reset)
{
    SequenceReset * reset = new SequenceReset;
    fillRecvHeader(reset->Header());

    *reset << new GapFillFlag(true)
           << new NewSeqNo(100);
    f8String tmp;
    reset->encode(tmp);
    initiator_test.ss->update_received();
    initiator_test.ss->process(tmp);

    EXPECT_EQ(unsigned(100), initiator_test.ss->get_next_receive_seq());
    recv_seq = 100;
}

TEST_F(sessionTest, handle_test_request)
{
    TestRequest * testreq = new TestRequest;
    fillRecvHeader(testreq->Header());

    *testreq << new TestReqID("hello");

    f8String tmp;
    testreq->encode(tmp);
    initiator_test.ss->update_received();
    initiator_test.ss->process(tmp);

    f8String output = getSingleMsg();

    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=0") !=  std::string::npos);

    clearOutputs();
}

TEST_F(sessionTest, send_test_request)
{
    //make sure test request is sent
    Tickval empty;
    initiator_test.ss->set_last_received(empty);
    initiator_test.ss->kickHBService();

    std::vector<f8String> outputs = getMsgs();

    f8String output = outputs.back();
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=1") !=  std::string::npos);
    EXPECT_EQ(States::st_test_request_sent, initiator_test.ss->getState());

    RegExp testID("112=(\\w+)");
    RegMatch match;
    f8String reqId;
    testID.SearchString(match, output, 2);
    testID.SubExpr(match, output, reqId, 0, 1);

    UTEST::Heartbeat hb;
    fillRecvHeader(hb.Header());
    hb << new TestReqID(reqId);
    f8String heartbeat;
    hb.encode(heartbeat);
    initiator_test.ss->process(heartbeat);
    EXPECT_EQ(States::st_continuous, initiator_test.ss->getState());

    //fail to response testRequest
    initiator_test.ss->kickHBService();
    initiator_test.ss->kickHBService();

    outputs = getMsgs();
    output = outputs.back();

    EXPECT_EQ(States::st_logoff_sent, initiator_test.ss->getState());
    EXPECT_TRUE(output.find("35=5") !=  std::string::npos);
    clearOutputs();
}


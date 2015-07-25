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
#include <fix8/f8config.h>

#ifdef FIX8_PROFILING_BUILD
#include <sys/gmon.h>
#endif

#include <errno.h>
#define F8MOCK_CONNECTION 1
#include <fix8/f8includes.hpp>
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

/*!class SessionID test
    \param sessionId test suit name
    \param sessionId test case name*/

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

/// a test session
class test_session : public FIX8::Session
{
    utest_Router _router;

public:

    /// Ctor
    test_session(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0,
        FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, sid, persist, logger, plogger)
    {
        _timer.clear();
        _timer.stop();
        _timer.join();
    }

    /// Dtor
    ~test_session(){}

	 /// All sessions must provide this method
    bool handle_application(const unsigned seqnum, const FIX8::Message *&msg)
    {
        return enforce(seqnum, msg) || msg->process(_router);
    }

    /// used to get session state
    States::SessionStates getState() {return _state;}

    /// used to get next receive seq
    unsigned get_next_receive_seq() {return _next_receive_seq;}

    /// used to set last received seq
    void set_last_received(Tickval& val) {_last_received = val;}

    ///start heartbeat service thread
    void kickHBService() {heartbeat_service();}
};

/// a session fixture to create and destroy test session
class session_fixture
{
public:
    MemoryPersister * per;
    Logger * pLogger;
    Logger * sLogger;
    test_session * ss;

    /// Ctor, create a test session with memory persister and pipe logger
    session_fixture()
    {
        SessionID id("FIX.4.2:A12345B->COMPARO");

#ifdef _MSC_VER
        ebitset<Logger::Flags> flag;
        flag.set(Logger::timestamp, true);
        flag.set(Logger::sequence, true);
        flag.set(Logger::thread, true);
        flag.set(Logger::direction, true);

        per = new MemoryPersister;
        sLogger = new FileLogger( "utest_slog.log", flag, Logger::Levels( Logger::All ) );
        pLogger = new FileLogger( "utest_plog.log", flag, Logger::Levels( Logger::All ) );
#else
        f8String cmd("|/bin/cat");
        ebitset<Logger::Flags> flag;
        flag.set(Logger::timestamp, true);
        flag.set(Logger::sequence, true);
        flag.set(Logger::thread, true);
        flag.set(Logger::direction, true);
        flag.set(Logger::append, true);
        flag.set(Logger::pipe, true);

        per = new MemoryPersister;
        sLogger = new PipeLogger(cmd, flag, Logger::Levels(Logger::All));
        pLogger = new PipeLogger(cmd, flag, Logger::Levels(Logger::All));
#endif // _MSC_VER

        ss = new test_session(ctx(), id, per, sLogger, pLogger);
    };

    /// Dtor
    ~session_fixture()
    {
        delete ss;
        delete per;
    };

};

/// a initiator fixture, inherited from session_fixture
class initiator_fixture : public session_fixture
{
public:
    ClientConnection * initiator;

    /// Ctor, create a test initiator connected to "127.0.0.1:80"
    initiator_fixture()
    {
        Poco::Net::SocketAddress addr("127.0.0.1:80");
        initiator = new ClientConnection(0, addr, *ss, pm_thread, false);
        initiator->connect();
        ss->start(initiator, false);
    };

    /// Dtor
    ~initiator_fixture()
    {
        ss->stop();
        delete initiator;
    };
};

/// gtest test suit event
class sessionTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
		initiator_test = new initiator_fixture();
        recv_seq = 1;
    }

    static void TearDownTestCase()
    {
		delete initiator_test;
    }
public:
    // MSVC: We can't have a static initiator here; we get bitten by
    // static order initialisation issues - specifically, the initiator
    // tries to construct a Logon message before the static members of
    // Logon (field maps, traist etc) have been initialised.
    static initiator_fixture* initiator_test;
    static unsigned recv_seq;
};

/// global initiator fixture
initiator_fixture* sessionTest::initiator_test = 0;

/// global incoming sequence number
unsigned sessionTest::recv_seq;

/*!helper to fill incoming message header
    \param header message header pointer
    \param seq sequence number*/
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

/*!helper to fill output message header
    \param header message header pointer*/

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

///helper to get the last output message
f8String getSingleMsg()
{
    if(sessionTest::initiator_test->initiator->_output.empty())
    {
        return "";
    }

    f8String output = sessionTest::initiator_test->initiator->_output[0];
    sessionTest::initiator_test->initiator->_output.clear();
    return output;
}

///helper to get all output messages
std::vector<f8String> getMsgs()
{
    std::vector<f8String> ret = sessionTest::initiator_test->initiator->_output;
    sessionTest::initiator_test->initiator->_output.clear();
    return ret;
}

///helper to clear cached output messages
void clearOutputs()
{
    sessionTest::initiator_test->initiator->_output.clear();
}

/*!helper to create a single new order and encode it to string
    \param seq sequence number
    \return new order entry in string format*/

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

/*!session logon test
    \param sessionTest test suit name
    \param logon test case name*/

TEST_F(sessionTest, logon)
{
    //initiator send logon
    f8String output = getSingleMsg();
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=A") !=  std::string::npos);
    EXPECT_EQ(States::st_logon_sent, initiator_test->ss->getState());

    //receive logon confirm
    Logon * logon = new Logon;
    fillRecvHeader(logon->Header());

    *logon << new HeartBtInt(5) << new EncryptMethod(0);
    f8String logon_str;
    logon->encode(logon_str);
    initiator_test->ss->update_received();
    initiator_test->ss->process(logon_str);

    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());
    EXPECT_EQ(unsigned(5), initiator_test->initiator->get_hb_interval());

    delete logon;

    clearOutputs();
}

/*!session resend request test
    \param sessionTest test suit name
    \param handle_resend_request test case name*/

TEST_F(sessionTest, handle_resend_request)
{
    //no message available
    ResendRequest * resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(4);
    f8String resend_str;
    resend->encode(resend_str);
    delete resend;

    initiator_test->ss->set_persister(0);
    initiator_test->ss->update_received();
    initiator_test->ss->process(resend_str);

    f8String output = getSingleMsg();

    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());
    EXPECT_TRUE(output.find("35=4") !=  std::string::npos);

    //ill resend request
    initiator_test->ss->set_persister(initiator_test->per);
    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(100) << new EndSeqNo(4);
    resend->encode(resend_str);
    delete resend;

    initiator_test->ss->update_received();
    initiator_test->ss->process(resend_str);

    output = getSingleMsg();

    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());
    EXPECT_TRUE(output.find("35=3") !=  std::string::npos);

    //have records
    unsigned next_send = initiator_test->ss->get_next_send_seq();

    Reject * rej = new Reject;
    fillSendHeader(rej->Header(), next_send);
    *rej << new RefSeqNum(100);
    f8String first;
    rej->encode(first);
    delete rej;
    initiator_test->per->put(next_send, first);
    initiator_test->per->put(++next_send, recv_seq);

    f8String second = composeOrder(next_send);
    initiator_test->per->put(next_send, second);
    initiator_test->per->put(++next_send, recv_seq);

    f8String third = composeOrder(next_send);
    initiator_test->per->put(next_send, third);
    initiator_test->per->put(++next_send, recv_seq);

    f8String four = composeOrder(next_send);
    initiator_test->per->put(next_send, four);
    initiator_test->per->put(++next_send, recv_seq);

    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(4);
    resend->encode(resend_str);
    delete resend;

    initiator_test->ss->update_received();
    initiator_test->ss->process(resend_str);

    std::vector<f8String> outputs = getMsgs();

    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());
    EXPECT_EQ(first, outputs[0]);
    EXPECT_EQ(second, outputs[1]);
    EXPECT_EQ(third, outputs[2]);
    EXPECT_EQ(four, outputs[3]);

    //have seq gap
    ++next_send; //skip 5
    f8String six = composeOrder(next_send);
    initiator_test->per->put(next_send, six);
    initiator_test->per->put(++next_send, recv_seq);

    resend = new ResendRequest;
    fillRecvHeader(resend->Header());
    *resend << new BeginSeqNo(1) << new EndSeqNo(6);
    resend->encode(resend_str);
    delete resend;

    initiator_test->ss->update_received();
    initiator_test->ss->process(resend_str);

    outputs = getMsgs();

    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());
    EXPECT_EQ(first, outputs[0]);
    EXPECT_EQ(second, outputs[1]);
    EXPECT_EQ(third, outputs[2]);
    EXPECT_EQ(four, outputs[3]);
    EXPECT_TRUE(outputs[4].find("35=4") !=  std::string::npos);
    EXPECT_EQ(six, outputs[5]);

    clearOutputs();
}

/*!session sequence rest test
    \param sessionTest test suit name
    \param handle_seq_reset test case name*/

TEST_F(sessionTest, handle_seq_reset)
{
    SequenceReset * reset = new SequenceReset;
    fillRecvHeader(reset->Header());

    *reset << new GapFillFlag(true)
           << new NewSeqNo(100);
    f8String tmp;
    reset->encode(tmp);
    initiator_test->ss->update_received();
    initiator_test->ss->process(tmp);

    EXPECT_EQ(unsigned(100), initiator_test->ss->get_next_receive_seq());
    recv_seq = 100;
}

/*!incoming test request test
    \param sessionTest test suit name
    \param handle_test_request test case name*/

TEST_F(sessionTest, handle_test_request)
{
    TestRequest * testreq = new TestRequest;
    fillRecvHeader(testreq->Header());

    *testreq << new TestReqID("hello");

    f8String tmp;
    testreq->encode(tmp);
    initiator_test->ss->update_received();
    initiator_test->ss->process(tmp);

    f8String output = getSingleMsg();

    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=0") !=  std::string::npos);

    clearOutputs();
}

/*!output test request test
    \param sessionTest test suit name
    \param send_test_request test case name*/

TEST_F(sessionTest, send_test_request)
{
    //make sure test request is sent
    Tickval empty;
    initiator_test->ss->set_last_received(empty);
    initiator_test->ss->kickHBService();

    std::vector<f8String> outputs = getMsgs();

    f8String output = outputs.back();
    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("35=1") !=  std::string::npos);
    EXPECT_EQ(States::st_test_request_sent, initiator_test->ss->getState());

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
    initiator_test->ss->process(heartbeat);
    EXPECT_EQ(States::st_continuous, initiator_test->ss->getState());

    //fail to response testRequest
    initiator_test->ss->kickHBService();
    initiator_test->ss->kickHBService();

    outputs = getMsgs();
    output = outputs.back();

    EXPECT_EQ(States::st_logoff_sent, initiator_test->ss->getState());
    EXPECT_TRUE(output.find("35=5") !=  std::string::npos);
    clearOutputs();
}


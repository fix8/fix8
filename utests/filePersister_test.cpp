// f8 headers
#include <f8headers.hpp>
#include <f8includes.hpp>
#include <persist.hpp>
#include <f8utils.hpp>
#include <limits>
#include <Poco/File.h>
#include <Poco/Path.h>
#include "utest_types.hpp"
#include "utest_router.hpp"
#include "utest_classes.hpp"
#include "gtest/gtest.h"

using namespace FIX8;
using namespace FIX8::UTEST;

class persist_fixture
{
    bool before, after;
    Poco::File directory;
    void unlink_directory()
    {
        if ( directory.exists() && directory.isDirectory() )
        {
            try
            {
                directory.remove(true);
            }
            catch(Poco::Exception &e)
            {
                std::cout << e.displayText() << std::endl;
            }
        }
    }

public:
    persist_fixture(f8String& curPath, bool before_clear = true, bool after_clear = false):
    before(before_clear),
    after(after_clear),
    directory(curPath + "/"+ "store")
    {
        if (before)
        {
            unlink_directory();
            directory.createDirectory();
        }
        filePer.initialise(curPath + "/" + "store", "utest_log");
    }

    ~persist_fixture()
    {
        if (after)
        {
            unlink_directory();
        }
    }

    FilePersister filePer;
};

f8String curPath = Poco::Path::current();

TEST(filePersister, create_file)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(exist("store/utest_log.idx"));
    EXPECT_TRUE(exist("store/utest_log"));
}

TEST(filePersister, seq_control)
{
    persist_fixture fixture(curPath, true, true);

    fixture.filePer.put(15, 10);
    fixture.filePer.put(19, 1000);

    unsigned sender_seq, target_seq;
    fixture.filePer.get(sender_seq, target_seq);

    EXPECT_EQ(unsigned(19), sender_seq);
    EXPECT_EQ(unsigned(1000), target_seq);

    fixture.filePer.put(0, 0);
    fixture.filePer.get(sender_seq, target_seq);
    EXPECT_EQ(unsigned(0), sender_seq);
    EXPECT_EQ(unsigned(0), target_seq);

    unsigned max = std::numeric_limits<unsigned>::max();
    fixture.filePer.put(max, max);
    fixture.filePer.get(sender_seq, target_seq);
    EXPECT_EQ(max, sender_seq);
    EXPECT_EQ(max, target_seq);
}

TEST(filePersister, message_store)
{
    persist_fixture fixture(curPath, true, true);

    EXPECT_FALSE(fixture.filePer.put(0, "this is zero"));

    EXPECT_TRUE(fixture.filePer.put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer.put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer.put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer.put(100, "this is one hundred"));

    EXPECT_FALSE(fixture.filePer.put(15, "this is fifteen"));

    f8String to;
    EXPECT_TRUE(fixture.filePer.get(100, to));
    EXPECT_EQ("this is one hundred", to);
    EXPECT_TRUE(fixture.filePer.get(15, to));
    EXPECT_EQ("this is fifteen", to);
    EXPECT_TRUE(fixture.filePer.get(16, to));
    EXPECT_EQ("this is sixteen", to);

    EXPECT_FALSE(fixture.filePer.get(0, to));
}

TEST(filePersister, get_last_seqnum)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer.put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer.put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer.put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer.put(100, "this is one hundred"));

    unsigned last;
    EXPECT_EQ(fixture.filePer.get_last_seqnum(last), unsigned(100));

    //sender, receive seq do nothing with last seq
    fixture.filePer.put(10000, 10);
    EXPECT_EQ(fixture.filePer.get_last_seqnum(last), unsigned(100));

    unsigned max = std::numeric_limits<unsigned>::max();
    EXPECT_TRUE(fixture.filePer.put(max, "this is max"));
    EXPECT_EQ(max, fixture.filePer.get_last_seqnum(last));

    EXPECT_TRUE(fixture.filePer.put(999, "this is 999"));
    EXPECT_EQ(max, fixture.filePer.get_last_seqnum(last));
}

TEST(filePersister, find_nearest_highest_seq)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer.put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer.put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer.put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer.put(100, "this is one hundred"));

    unsigned max = std::numeric_limits<unsigned>::max();
    EXPECT_EQ(unsigned(1), fixture.filePer.find_nearest_highest_seqnum(0, max));
    EXPECT_EQ(unsigned(0), fixture.filePer.find_nearest_highest_seqnum(0, 0));
    EXPECT_EQ(unsigned(0), fixture.filePer.find_nearest_highest_seqnum(2, 2));
    EXPECT_EQ(unsigned(15), fixture.filePer.find_nearest_highest_seqnum(2, 16));
    EXPECT_EQ(unsigned(0), fixture.filePer.find_nearest_highest_seqnum(max, 10));
}

class check_session : public Session
{
public:
    check_session(const F8MetaCntx& ctx):
        Session(ctx)
    {
        _connection = new Connection(NULL, *this);
    };

    ~check_session()
    {
        _timer.clear();
        _timer.schedule(_hb_processor, 0);
        _timer.join();
    };

    virtual bool retrans_callback(const SequencePair& with, RetransmissionContext& rctx)
    {
        _resends.insert(std::make_pair(with.first, with.second));
        return true;
    }

    std::map<unsigned, f8String> _resends;
};

TEST(filePersister, resend_get)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer.put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer.put(2, "this is two"));
    EXPECT_TRUE(fixture.filePer.put(3, "this is three"));
    EXPECT_TRUE(fixture.filePer.put(4, "this is four"));
    EXPECT_TRUE(fixture.filePer.put(5, "this is five"));


    check_session session(UTEST::ctx);
    Connection connection(NULL, session);

    fixture.filePer.get(1, 5, session, &Session::retrans_callback);

    EXPECT_EQ(size_t(6), session._resends.size());

    EXPECT_EQ("", session._resends[0]);
    EXPECT_EQ("this is one", session._resends[1]);
    EXPECT_EQ("this is two", session._resends[2]);
    EXPECT_EQ("this is three", session._resends[3]);
    EXPECT_EQ("this is four", session._resends[4]);
    EXPECT_EQ("this is five", session._resends[5]);

}

//read from persist file
TEST(filePersister, read_from_file)
{
    {
        persist_fixture prepare_fixture(curPath, true, false);
        prepare_fixture.filePer.put(1, 1);

        prepare_fixture.filePer.put(1, "this is one");
        prepare_fixture.filePer.put(2, 1);

        prepare_fixture.filePer.put(2, "this is two");
        prepare_fixture.filePer.put(3, 1);

        prepare_fixture.filePer.put(3, "this is three");
        prepare_fixture.filePer.put(4, 1);

        prepare_fixture.filePer.put(4, "this is four");
        prepare_fixture.filePer.put(5, 1);


        prepare_fixture.filePer.put(5, 100);
    }


    persist_fixture read_fixture(curPath, false, true);
    unsigned sender_seq, target_seq;
    read_fixture.filePer.get(sender_seq, target_seq);
    EXPECT_EQ(unsigned(5), sender_seq);
    EXPECT_EQ(unsigned(100), target_seq);

    f8String to;
    EXPECT_TRUE(read_fixture.filePer.get(1, to));
    EXPECT_EQ("this is one", to);
    EXPECT_TRUE(read_fixture.filePer.get(2, to));
    EXPECT_EQ("this is two", to);
    EXPECT_TRUE(read_fixture.filePer.get(3, to));
    EXPECT_EQ("this is three", to);
    EXPECT_TRUE(read_fixture.filePer.get(4, to));
    EXPECT_EQ("this is four", to);
}
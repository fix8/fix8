//-----------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------
// f8 headers
#include <f8headers.hpp>
#include <fix8/f8includes.hpp>
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

/// A helper class to create/remove persist file directory
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

    /*! Ctor
        \param curPath log persist directory path
        \param before_clear will remove existed persist files and folder in Ctor when it is set to true
        \param after_clear will remove existed persist files and folder in Dtor when it is set to true*/

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

        filePer = new FilePersister();
        filePer->initialise(curPath + "/" + "store", "utest_log");
    }

    /// Dtor
    ~persist_fixture()
    {
	// MSVC: We have to delete the filePersister before we
	// unlink the directory, otherwise we get a file sharing
	// violation
        if (filePer)
                delete filePer;

        if (after)
        {
            unlink_directory();
        }
    }

    /// file persister
    FilePersister* filePer;
};

/// current working path
f8String curPath = Poco::Path::current();


/*! persist file creation test
    \param filePersister test suit name
    \param create_file test case name*/

TEST(filePersister, create_file)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(exist("store/utest_log.idx"));
    EXPECT_TRUE(exist("store/utest_log"));
}

/*! sequence number test
    \param filePersister test suit name
    \param seq_control test case name*/

TEST(filePersister, seq_control)
{
    persist_fixture fixture(curPath, true, true);

    fixture.filePer->put(15, 10);
    fixture.filePer->put(19, 1000);

    unsigned sender_seq, target_seq;
    fixture.filePer->get(sender_seq, target_seq);

    EXPECT_EQ(unsigned(19), sender_seq);
    EXPECT_EQ(unsigned(1000), target_seq);

    fixture.filePer->put(0, 0);
    fixture.filePer->get(sender_seq, target_seq);
    EXPECT_EQ(unsigned(0), sender_seq);
    EXPECT_EQ(unsigned(0), target_seq);

    unsigned max = std::numeric_limits<unsigned>::max();
    fixture.filePer->put(max, max);
    fixture.filePer->get(sender_seq, target_seq);
    EXPECT_EQ(max, sender_seq);
    EXPECT_EQ(max, target_seq);
}

/*! message persist test
    \param filePersister test suit name
    \param message_store test case name*/


TEST(filePersister, message_store)
{
    persist_fixture fixture(curPath, true, true);

    EXPECT_FALSE(fixture.filePer->put(0, "this is zero"));

    EXPECT_TRUE(fixture.filePer->put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer->put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer->put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer->put(100, "this is one hundred"));

    EXPECT_FALSE(fixture.filePer->put(15, "this is fifteen"));

    f8String to;
    EXPECT_TRUE(fixture.filePer->get(100, to));
    EXPECT_EQ("this is one hundred", to);
    EXPECT_TRUE(fixture.filePer->get(15, to));
    EXPECT_EQ("this is fifteen", to);
    EXPECT_TRUE(fixture.filePer->get(16, to));
    EXPECT_EQ("this is sixteen", to);

    EXPECT_FALSE(fixture.filePer->get(0, to));
}

/*! last sequence number test
    \param filePersister test suit name
    \param get_last_seqnum test case name*/

TEST(filePersister, get_last_seqnum)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer->put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer->put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer->put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer->put(100, "this is one hundred"));

    unsigned last;
    EXPECT_EQ(fixture.filePer->get_last_seqnum(last), unsigned(100));

    //sender, receive seq do nothing with last seq
    fixture.filePer->put(10000, 10);
    EXPECT_EQ(fixture.filePer->get_last_seqnum(last), unsigned(100));

    unsigned max = std::numeric_limits<unsigned>::max();
    EXPECT_TRUE(fixture.filePer->put(max, "this is max"));
    EXPECT_EQ(max, fixture.filePer->get_last_seqnum(last));

    EXPECT_TRUE(fixture.filePer->put(999, "this is 999"));
    EXPECT_EQ(max, fixture.filePer->get_last_seqnum(last));
}

/*! test function FilePersister::find_nearest_highest_seq
    \param filePersister test suit name
    \param find_nearest_highest_seq test case name*/

TEST(filePersister, find_nearest_highest_seq)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer->put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer->put(15, "this is fifteen"));
    EXPECT_TRUE(fixture.filePer->put(16, "this is sixteen"));
    EXPECT_TRUE(fixture.filePer->put(100, "this is one hundred"));

    unsigned max = std::numeric_limits<unsigned>::max();
    EXPECT_EQ(unsigned(1), fixture.filePer->find_nearest_highest_seqnum(0, max));
    EXPECT_EQ(unsigned(0), fixture.filePer->find_nearest_highest_seqnum(0, 0));
    EXPECT_EQ(unsigned(0), fixture.filePer->find_nearest_highest_seqnum(2, 2));
    EXPECT_EQ(unsigned(15), fixture.filePer->find_nearest_highest_seqnum(2, 16));
    EXPECT_EQ(unsigned(0), fixture.filePer->find_nearest_highest_seqnum(max, 10));
}

/// A helper session to check retransmitted messages

class check_session : public Session
{
	Poco::Net::SocketAddress _addr;

public:

    /// Ctor
    check_session(const F8MetaCntx& ctx):
        Session(ctx)
    {
		 _connection = new Connection(0, _addr, *this, pm_thread, 10, false);
    };

    /// Dtor
    ~check_session()
    {
        _timer.clear();
        _timer.schedule(_hb_processor, 0);
        _timer.join();
    };

    /*! retrans_callback overload
            add resent messages to a local map for future check*/
    virtual bool retrans_callback(const SequencePair& with, RetransmissionContext& rctx)
    {
        _resends.insert(std::make_pair(with.first, with.second));
        return true;
    }

	 /// All sessions must provide this method
	 bool handle_application(const unsigned seqnum, const Message *&msg) { return true; }

    std::map<unsigned, f8String> _resends;
};

/*!test for getting a bundle of persisted messages.
    \param filePersister test suit name
    \param resend_get test case name*/

TEST(filePersister, resend_get)
{
    persist_fixture fixture(curPath, true, true);
    EXPECT_TRUE(fixture.filePer->put(1, "this is one"));
    EXPECT_TRUE(fixture.filePer->put(2, "this is two"));
    EXPECT_TRUE(fixture.filePer->put(3, "this is three"));
    EXPECT_TRUE(fixture.filePer->put(4, "this is four"));
    EXPECT_TRUE(fixture.filePer->put(5, "this is five"));


	 Poco::Net::SocketAddress _addr;
    check_session session(UTEST::ctx());
    Connection connection(0, _addr, session, pm_thread, 10, false);

    fixture.filePer->get(1, 5, session, &Session::retrans_callback);

    EXPECT_EQ(size_t(6), session._resends.size());

    EXPECT_EQ("", session._resends[0]);
    EXPECT_EQ("this is one", session._resends[1]);
    EXPECT_EQ("this is two", session._resends[2]);
    EXPECT_EQ("this is three", session._resends[3]);
    EXPECT_EQ("this is four", session._resends[4]);
    EXPECT_EQ("this is five", session._resends[5]);

}

/*!persist file read test
    \param filePersister test suit name
    \param read_from_file test case name*/

TEST(filePersister, read_from_file)
{
    {
        persist_fixture prepare_fixture(curPath, true, false);
        prepare_fixture.filePer->put(1, 1);

        prepare_fixture.filePer->put(1, "this is one");
        prepare_fixture.filePer->put(2, 1);

        prepare_fixture.filePer->put(2, "this is two");
        prepare_fixture.filePer->put(3, 1);

        prepare_fixture.filePer->put(3, "this is three");
        prepare_fixture.filePer->put(4, 1);

        prepare_fixture.filePer->put(4, "this is four");
        prepare_fixture.filePer->put(5, 1);


        prepare_fixture.filePer->put(5, 100);
    }


    persist_fixture read_fixture(curPath, false, true);
    unsigned sender_seq, target_seq;
    read_fixture.filePer->get(sender_seq, target_seq);
    EXPECT_EQ(unsigned(5), sender_seq);
    EXPECT_EQ(unsigned(100), target_seq);

    f8String to;
    EXPECT_TRUE(read_fixture.filePer->get(1, to));
    EXPECT_EQ("this is one", to);
    EXPECT_TRUE(read_fixture.filePer->get(2, to));
    EXPECT_EQ("this is two", to);
    EXPECT_TRUE(read_fixture.filePer->get(3, to));
    EXPECT_EQ("this is three", to);
    EXPECT_TRUE(read_fixture.filePer->get(4, to));
    EXPECT_EQ("this is four", to);
}

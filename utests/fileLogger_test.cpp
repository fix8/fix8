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
#include "f8headers.hpp"
#include <fix8/f8includes.hpp>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Thread.h>
#include <Poco/StringTokenizer.h>
#include "gtest/gtest.h"
#include "utest_types.hpp"
#include "utest_router.hpp"
#include "utest_classes.hpp"

using namespace FIX8;
using namespace FIX8::UTEST;

/// A helper class to create/remove log directory
class log_fixture
{
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
        \param curPath log file directory path */
    log_fixture(f8String& curPath):
    directory(curPath + "/"+ "log")
    {
        unlink_directory();
        directory.createDirectory();
    }

    /// Dtor
    ~log_fixture()
    {
        unlink_directory();
    }
};

/*! helper function to test log line
    \param str actual result string
    \param seq expected seq
    \param thread expected thread number
    \param direction expected direction
    \param message expected log string*/
void test_log_line(f8String& str, f8String seq, f8String thread, f8String direction,
                   f8String message)
{
    Poco::StringTokenizer token(str, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY);

    EXPECT_EQ(seq, token[0]);
    EXPECT_EQ(thread, token[1]);
    EXPECT_EQ(direction, token[2]);
    EXPECT_EQ(size_t(10), token[3].length());
    EXPECT_EQ(size_t(18), token[4].length());
    EXPECT_EQ(message, token[5]);
}

/*! log file creation test
    \param filelogger test suit name
    \param create_logfile test case name*/

TEST(filelogger, create_logfile)
{
    f8String curPath = Poco::Path::current();
    log_fixture fixture(curPath);

    ebitset<Logger::Flags> flag;
    flag.set(Logger::timestamp, true);
    flag.set(Logger::sequence, true);
    flag.set(Logger::thread, true);
    flag.set(Logger::direction, true);

    FileLogger logger("log/utest_log.log", flag);

    EXPECT_TRUE(exist("log/utest_log.log"));
    logger.send("first_message_out", 0);
    logger.send("first_message_in", 1);
    logger.send("second_message_out", 0);
    logger.send("third_message_out", 0);

    //wait until log file have been written
    hypersleep<h_milliseconds>(50);
    logger.stop();

    std::ifstream reader;
    reader.open("log/utest_log.log");

    f8String output;
    std::vector<f8String> lines;
    while(getline(reader, output, '\n'))
    {
        lines.push_back(output);
    }

    EXPECT_EQ(size_t(4), lines.size());
    test_log_line(lines[0], "0000001", "A", "out", "first_message_out");
    test_log_line(lines[1], "0000001", "A", "in", "first_message_in");
    test_log_line(lines[2], "0000002", "A", "out", "second_message_out");
    test_log_line(lines[3], "0000003", "A", "out", "third_message_out");
}

/*! log file rotate test
    \param filelogger test suit name
    \param rotate test case name*/

TEST(filelogger, rotate)
{
    f8String curPath = Poco::Path::current();
    log_fixture fixture(curPath);

    ebitset<Logger::Flags> flag;
    flag.set(Logger::timestamp, true);
    flag.set(Logger::sequence, true);
    flag.set(Logger::thread, true);
    flag.set(Logger::direction, true);

    FileLogger logger("log/utest_log.log", flag, 3);
    EXPECT_TRUE(exist("log/utest_log.log"));

    logger.rotate(true);
    EXPECT_TRUE(exist("log/utest_log.log"));
    EXPECT_TRUE(exist("log/utest_log.log.1"));

    logger.rotate(true);
    EXPECT_TRUE(exist("log/utest_log.log"));
    EXPECT_TRUE(exist("log/utest_log.log.1"));
    EXPECT_TRUE(exist("log/utest_log.log.2"));

    logger.rotate(true);
    EXPECT_TRUE(exist("log/utest_log.log"));
    EXPECT_TRUE(exist("log/utest_log.log.1"));
    EXPECT_TRUE(exist("log/utest_log.log.2"));
    EXPECT_TRUE(exist("log/utest_log.log.3"));

    logger.rotate(true);
    EXPECT_TRUE(exist("log/utest_log.log"));
    EXPECT_TRUE(exist("log/utest_log.log.1"));
    EXPECT_TRUE(exist("log/utest_log.log.2"));
    EXPECT_TRUE(exist("log/utest_log.log.3"));

    logger.stop();
}

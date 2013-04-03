// f8 headers
#include <f8headers.hpp>
#include <f8includes.hpp>
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

class log_fixture
{
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
    log_fixture(f8String& curPath):
    directory(curPath + "/"+ "log")
    {
        unlink_directory();
        directory.createDirectory();
    }

    ~log_fixture()
    {
        unlink_directory();
    }
    Poco::File directory;
};

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

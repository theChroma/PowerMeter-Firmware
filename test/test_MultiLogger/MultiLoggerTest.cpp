#include "Logger/MultiLogger/MultiLogger.h"

#include <gtest/gtest.h>
#include <sstream>
#include <fstream>

const std::string testString = "test123!";

TEST(MultiLoggerTest, logAll)
{
    std::stringstream testStream1;
    std::stringstream testStream2;
    MultiLogger uut({
        LogStream(LogLevel::Error, LogLevel::Verbose, &testStream1, false),
        LogStream(LogLevel::Error, LogLevel::Verbose, &testStream2, false),
    });
    uut[LogLevel::Verbose] << testString;
    EXPECT_EQ(testString, testStream1.str());
    EXPECT_EQ(testString, testStream2.str());
}

TEST(MultiLoggerTest, showLevel)
{
    std::stringstream testStream;
    MultiLogger uut({LogStream(LogLevel::Error, LogLevel::Verbose, &testStream, true)});

    uut[LogLevel::Error] << testString;
    EXPECT_EQ(std::string("[ERROR] ") + testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Warning] << testString;
    EXPECT_EQ(std::string("[WARNING] ") + testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Debug] << testString;
    EXPECT_EQ(std::string("[DEBUG] ") + testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Info] << testString;
    EXPECT_EQ(std::string("[INFO] ") + testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Verbose] << testString;
    EXPECT_EQ(std::string("[VERBOSE] ") + testString, testStream.str());
    testStream.str("");
}

TEST(MultiLoggerTest, minAndMaxLevel)
{
    std::stringstream testStream;
    MultiLogger uut({LogStream(LogLevel::Warning, LogLevel::Debug, &testStream, false)});

    uut[LogLevel::Error] << testString;
    EXPECT_EQ("", testStream.str());
    testStream.str("");

    uut[LogLevel::Warning] << testString;
    EXPECT_EQ(testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Info] << testString;
    EXPECT_EQ(testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Debug] << testString;
    EXPECT_EQ(testString, testStream.str());
    testStream.str("");

    uut[LogLevel::Verbose] << testString;
    EXPECT_EQ("", testStream.str());
    testStream.str("");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
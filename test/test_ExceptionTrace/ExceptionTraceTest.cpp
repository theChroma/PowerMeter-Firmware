#include "ExceptionTrace/ExceptionTrace.h"

#include <exception>
#include <iostream>
#include <gtest/gtest.h>

void c()
{
    try
    {
        throw std::runtime_error("foo");
    }
    catch(...)
    {
        ExceptionTrace::trace("c");
        throw;
    }
}

void b()
{
    try
    {
        c();
    }
    catch(...)
    {
        ExceptionTrace::trace("b");
        throw;
    }
}

void a()
{
    try
    {
        b();
    }
    catch(...)
    {
        ExceptionTrace::trace("a");
        throw;
    }
}

void b1()
{
    try
    {
        c();
    }
    catch(const std::runtime_error& e)
    {
        EXPECT_TRUE(true) << "Caught runtime_error" << std::endl;
    }

}

void a1()
{
    try
    {
        b1();
    }
    catch(...)
    {
        ExceptionTrace::trace("a");
        throw;
    }

}


TEST(ExceptionTraceTest, what)
{
    try
    {
        a();
    }
    catch(...)
    {
        EXPECT_EQ("a\r\nxb\r\nxxc\r\nxxxfoo\r\n", ExceptionTrace::what(true, 1, 'x'));
    }
}

TEST(ExceptionTraceTest, get)
{
    try
    {
        a();
    }
    catch(...)
    {
        EXPECT_EQ(ExceptionTrace::Traces({"a", "b", "c", "foo"}), ExceptionTrace::get());
    }
}

TEST(ExceptionTraceTest, catchInbetween)
{
    try
    {
        a1();
    }
    catch(...)
    {
        FAIL() << "Should have been caught yet" << std::endl;
    }
}

TEST(ExceptionTraceTest, clear)
{
    ExceptionTrace::trace("xyz");
    ExceptionTrace::clear();
    EXPECT_EQ("", ExceptionTrace::what());
}

TEST(ExceptionTraceTest, clearOnWhat)
{
    ExceptionTrace::trace("xyz");
    ExceptionTrace::what();
    EXPECT_EQ("", ExceptionTrace::what());

    ExceptionTrace::trace("xyz");
    ExceptionTrace::what(false);
    EXPECT_EQ("xyz\r\n", ExceptionTrace::what());
}

TEST(ExceptionTraceTest, clearOnGet)
{
    ExceptionTrace::trace("xyz");
    ExceptionTrace::what();
    EXPECT_EQ(ExceptionTrace::Traces({}), ExceptionTrace::get());

    ExceptionTrace::trace("xyz");
    ExceptionTrace::get(false);
    EXPECT_EQ(ExceptionTrace::Traces({"xyz"}), ExceptionTrace::get());
}


int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
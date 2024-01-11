#include "Version/Version.h"

#include <gtest/gtest.h>
#include <sstream>

using PM::Version;

TEST(VersionTest, parseAndPrint)
{
    std::string version("1.2.3");
    Version uut = Version(version);
    EXPECT_EQ(version, static_cast<std::string>(uut));
    std::stringstream ss;
    ss << uut;
    EXPECT_EQ(version, ss.str());
}

TEST(VersionTest, parseBad)
{
    EXPECT_ANY_THROW(Version("1-2-3"));
}

TEST(VersionTest, equality)
{
    EXPECT_TRUE(Version(1, 2, 3) == Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 3) == Version(1, 2, 5));
    EXPECT_TRUE(Version(1, 2, 3) != Version(1, 2, 5));
    EXPECT_FALSE(Version(1, 2, 3) != Version(1, 2, 3));
}

TEST(VersionTest, greater)
{
    EXPECT_TRUE(Version(1, 2, 5) > Version(1, 2, 3));
    EXPECT_TRUE(Version(1, 3, 0) > Version(1, 2, 3));
    EXPECT_TRUE(Version(2, 0, 0) > Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 3) > Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 2) > Version(1, 2, 3));
}

TEST(VersionTest, greaterOrEqual)
{
    EXPECT_TRUE(Version(1, 2, 5) >= Version(1, 2, 3));
    EXPECT_TRUE(Version(1, 3, 0) >= Version(1, 2, 3));
    EXPECT_TRUE(Version(2, 0, 0) >= Version(1, 2, 3));
    EXPECT_TRUE(Version(1, 2, 3) >= Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 2) >= Version(1, 2, 3));
}

TEST(VersionTest, less)
{
    EXPECT_TRUE(Version(1, 2, 3) < Version(1, 2, 5));
    EXPECT_TRUE(Version(1, 2, 3) < Version(1, 3, 0));
    EXPECT_TRUE(Version(1, 2, 3) < Version(2, 0, 0));
    EXPECT_FALSE(Version(1, 2, 3) < Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 3) < Version(1, 2, 2));
}

TEST(VersionTest, lessOrEqual)
{
    EXPECT_TRUE(Version(1, 2, 3) <= Version(1, 2, 5));
    EXPECT_TRUE(Version(1, 2, 3) <= Version(1, 3, 0));
    EXPECT_TRUE(Version(1, 2, 3) <= Version(2, 0, 0));
    EXPECT_TRUE(Version(1, 2, 3) <= Version(1, 2, 3));
    EXPECT_FALSE(Version(1, 2, 3) <= Version(1, 2, 2));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
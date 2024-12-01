#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"
#include "MockFile.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <gtest/gtest.h>

const json testJson = {"foo", 43};

struct BackedUpJsonResourceTest : public testing::Test
{
    MockFile* mockFileA = new MockFile("MockFileA path", "MockFileA name");
    MockFile* mockFileB = new MockFile("MockFileA path", "MockFileA name");
    BackedUpJsonResource uut = BackedUpJsonResource(
        BasicJsonResource(std::unique_ptr<Filesystem::File>(mockFileA)),
        BasicJsonResource(std::unique_ptr<Filesystem::File>(mockFileB))
    );
};


TEST_F(BackedUpJsonResourceTest, serialize)
{
    mockFileA->lastWriteTimestamp = 1;
    mockFileB->lastWriteTimestamp = 2;
    // Should serialize to A now
    uut.serialize(testJson);
    EXPECT_EQ(testJson, json::parse(mockFileA->stream));
    EXPECT_TRUE(mockFileB->stream.str().empty());
}


TEST_F(BackedUpJsonResourceTest, deserialize)
{
    mockFileB->stream << testJson;
    mockFileA->lastWriteTimestamp = 1;
    mockFileB->lastWriteTimestamp = 2;
    // Should deserialize from B now
    EXPECT_EQ(testJson, uut.deserialize());
}


TEST_F(BackedUpJsonResourceTest, corruptedFile)
{
    mockFileA->stream << testJson;
    mockFileB->stream << "This is corrupted";
    mockFileA->lastWriteTimestamp = 1;
    mockFileB->lastWriteTimestamp = 2;
    // Should deserialize from A now
    EXPECT_EQ(testJson, uut.deserialize());
    EXPECT_ANY_THROW(json::parse(mockFileB->stream));
    uut.serialize(testJson);
    std::cout << mockFileB->stream.str() << std::endl;
    EXPECT_EQ(testJson, json::parse(mockFileB->stream));
}


int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
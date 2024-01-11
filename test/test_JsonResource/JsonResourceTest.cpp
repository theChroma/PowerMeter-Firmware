#include "JsonResource/JsonResource.h"
#include <gtest/gtest.h>
#include <filesystem>

using namespace PM;

const std::string testFilePath = "JsonResourceTest.json";
const json::json_pointer testJsonPointer = "/1/bar"_json_pointer;
const json testData = {
    {"foo", 1.0},
    {"bar", 2},
    {"baz", {"sdf"}},
    {"buz", "text"}
};

struct JsonResourceTest : public testing::Test
{
    JsonResource uut = JsonResource(testFilePath, testJsonPointer);

    void TearDown() override
    {
        std::filesystem::remove(testFilePath);
    }
};


TEST_F(JsonResourceTest, empty)
{
    JsonResource empty;
    EXPECT_EQ("", empty.getFilePath());
    EXPECT_EQ(""_json_pointer, empty.getJsonPointer());
    EXPECT_EQ("", static_cast<std::string>(empty));

    empty = JsonResource("");
    EXPECT_EQ("", empty.getFilePath());
    EXPECT_EQ(""_json_pointer, empty.getJsonPointer());
    EXPECT_EQ("", static_cast<std::string>(empty));
}

TEST_F(JsonResourceTest, construct)
{
    JsonResource testURI("foo/bar.json#/hello/1/world");
    EXPECT_EQ("foo/bar.json", testURI.getFilePath());
    EXPECT_EQ("/hello/1/world"_json_pointer, testURI.getJsonPointer());
}

TEST_F(JsonResourceTest, checkSettersAndGetters)
{
    uut.setFilePath("JsonResourceTest1.json");
    EXPECT_EQ("JsonResourceTest1.json", uut.getFilePath());

    uut.setJsonPointer("/another/json/ptr"_json_pointer);
    EXPECT_EQ("/another/json/ptr"_json_pointer, uut.getJsonPointer());
}

TEST_F(JsonResourceTest, serializeDeserialize)
{
    uut.serialize(testData);
    EXPECT_EQ(testData, uut.deserialize());

    // uut.serialize(json());
    // EXPECT_EQ(json(), uut.deserialize());
}


TEST_F(JsonResourceTest, deserializeNonexistingJsonPointer)
{
    uut.serialize(testData);
    uut.setJsonPointer("/0/does/not/exist"_json_pointer);
    EXPECT_THROW(uut.deserialize(), json::exception);
}

TEST_F(JsonResourceTest, deserializeNonexistingFile)
{
    uut.setFilePath("doesnot.exist");
    EXPECT_THROW(uut.deserialize(), std::runtime_error);
}


TEST_F(JsonResourceTest, checkAppendAndAssign)
{
    JsonResource empty;
    empty /= "/foo/bar"_json_pointer;
    EXPECT_EQ("/foo/bar"_json_pointer, empty.getJsonPointer());
    EXPECT_EQ("", empty.getFilePath());

    uut /= "/a/b"_json_pointer;
    EXPECT_EQ(testJsonPointer / "/a/b"_json_pointer, uut.getJsonPointer());
}


TEST_F(JsonResourceTest, checkAppend)
{
    JsonResource empty;
    JsonResource actual;
    actual = empty / "/foo/bar"_json_pointer;
    EXPECT_EQ("/foo/bar"_json_pointer, actual.getJsonPointer());

    actual = uut / "/a/b"_json_pointer;
    EXPECT_EQ(testJsonPointer / "/a/b"_json_pointer, actual.getJsonPointer());
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
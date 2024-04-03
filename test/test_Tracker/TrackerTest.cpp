#include "Tracker/Tracker.h"
#include "MockClock.h"
#include "ExceptionTrace/ExceptionTrace.h"

#include <gtest/gtest.h>

using namespace PM;

constexpr time_t duration_s = 3600;
constexpr size_t sampleCount = 60;


struct TrackerTest : public testing::Test
{
    void SetUp() override
    {
        std::filesystem::create_directory("TrackerTest");
    }

    void TearDown() override
    {
        std::filesystem::remove_all("TrackerTest");
    }

    MockClock mockClock = MockClock();
    Tracker uut = Tracker(
        "Test Tracker",
        duration_s,
        sampleCount,
        mockClock,
        JsonResource("TrackerTest/data.json"),
        JsonResource("TrackerTest/timestamps.json#/lastInput"),
        JsonResource("TrackerTest/timestamps.json#/lastSample"),
        AverageAccumulator(JsonResource("TrackerTest/accumulator.json"))
    );
};

TEST_F(TrackerTest, checkGetDataOfEmptyTracker)
{
    try
    {
        json expectedData;
        expectedData["data"] = json::array_t();
        expectedData["duration_s"] = 3600;
        expectedData["sampleCount"] = 60;
        expectedData["title"] = "Test Tracker";
        EXPECT_EQ(expectedData, uut.getData());
    }
    catch(...)
    {
        FAIL() << ExceptionTrace::what() << std::endl;
    }
}


TEST_F(TrackerTest, shouldFillWithZeroWhilePowerDown)
{
    try
    {
        for(size_t i = 0; i < 1000; i++)
        {
            uut.track(1.0f);
            mockClock.tick();
        }
        mockClock.tick(1000);
        for(size_t i = 0; i < 1621; i++)
        {
            uut.track(1.0f);
            mockClock.tick();
        }
        json data = uut.getData().at("data");
        EXPECT_EQ(data.size(), sampleCount);
        json expected = {
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0,
            1.0
        };
        EXPECT_EQ(expected, data);
    }
    catch(...)
    {
        FAIL() << ExceptionTrace::what() << std::endl;
    }
}


int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
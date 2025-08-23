#include "Tracker/Tracker.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MockClock.h"
#include "MockFile.h"
#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"

#include <gtest/gtest.h>


constexpr time_t duration_s = 3600;
constexpr size_t sampleCount = 60;


struct FileTrackerTest : public testing::Test
{
    MockClock mockClock = MockClock();
    Tracker uut = Tracker(
        "Test Tracker",
        duration_s,
        sampleCount,
        &mockClock,
        std::make_unique<BackedUpJsonResource>(
            BasicJsonResource(std::make_unique<MockFile>("data path A", "data name A")),
            BasicJsonResource(std::make_unique<MockFile>("data path B", "data name B"))
        ),
        std::make_unique<BackedUpJsonResource>(
            BasicJsonResource(std::make_unique<MockFile>("last input path A", "last input name A")),
            BasicJsonResource(std::make_unique<MockFile>("last input path B", "last input name B"))
        ),
        std::make_unique<BackedUpJsonResource>(
            BasicJsonResource(std::make_unique<MockFile>("last sample path A", "last sample name A")),
            BasicJsonResource(std::make_unique<MockFile>("last sample path B", "last sample name B"))
        ),
        AverageAccumulator(
            std::make_unique<BackedUpJsonResource>(
                BasicJsonResource(std::make_unique<MockFile>("accumulator path A", "accumulator name A")),
                BasicJsonResource(std::make_unique<MockFile>("accumulator path B", "accumulator name B"))
            )
        )
    );
};

TEST_F(FileTrackerTest, checkGetDataOfEmptyTracker)
{
    try
    {
        json expectedData;
        expectedData["data"] = nullptr;
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


TEST_F(FileTrackerTest, shouldFillWithZeroWhilePowerDown)
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

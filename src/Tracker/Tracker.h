#pragma once

#include "Clock/Clock.h"
#include "JsonResource/JsonResource.h"
#include "AverageAccumulator/AverageAccumulator.h"
#include <unordered_map>
#include <memory>

class Tracker
{
public:
    Tracker(
        std::string title,
        time_t duration_s,
        size_t sampleCount,
        const Clock* clock,
        std::unique_ptr<JsonResource> dataResource,
        std::unique_ptr<JsonResource> lastInputResource,
        std::unique_ptr<JsonResource> lastSampleResource,
        AverageAccumulator accumulator
    ) noexcept;
    void track(float value);
    json getData() const;
    void setData(const json& data);
    void erase();

private:
    void updateData(const std::vector<float>& newValues);
    time_t getTimestamp(JsonResource& timestampResource) const;

    std::string m_title;
    time_t m_duration_s;
    size_t m_sampleCount;
    const Clock* m_clock;
    std::unique_ptr<JsonResource> m_dataResource;
    std::unique_ptr<JsonResource> m_lastInputResource;
    std::unique_ptr<JsonResource> m_lastSampleResource;
    AverageAccumulator m_accumulator;
};

using TrackerMap = std::unordered_map<std::string, Tracker>;
#pragma once

#include "Clock/Clock.h"
#include "JsonResource/JsonResource.h"
#include "AverageAccumulator/AverageAccumulator.h"
#include <unordered_map>

namespace PM
{
    class Tracker
    {
    public:
        Tracker(
            const std::string& title,
            time_t duration_s,
            size_t sampleCount,
            const Clock& clock,
            const JsonResource& dataResource,
            const JsonResource& lastInputResource,
            const JsonResource& lastSampleResource,
            const AverageAccumulator& accumulator
        ) noexcept;
        void track(float value);
        json getData() const;
        void setData(const json& data);
        void erase();

    private:
        void updateData(float value);
        time_t getTimestamp(const JsonResource& timestampResource) const;

        std::string m_title;
        time_t m_duration_s;
        size_t m_sampleCount;
        const Clock& m_clock;
        JsonResource m_dataResource;
        JsonResource m_lastInputResource;
        JsonResource m_lastSampleResource;
        AverageAccumulator m_accumulator;
    };

    using TrackerMap = std::unordered_map<std::string, Tracker>;
}
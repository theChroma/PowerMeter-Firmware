#include "Tracker.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "Logger/Logger.h"
#include "ScopeProfiler/ScopeProfiler.h"

#include <math.h>
#include <sstream>

using PM::Tracker;

Tracker::Tracker(
    const std::string& title,
    time_t duration_s,
    size_t sampleCount,
    const Clock& clock,
    std::shared_ptr<JsonResource> dataResource,
    std::shared_ptr<JsonResource> lastInputResource,
    std::shared_ptr<JsonResource> lastSampleResource,
    const AverageAccumulator& accumulator
) noexcept :
    m_title(title),
    m_duration_s(duration_s),
    m_sampleCount(sampleCount),
    m_clock(clock),
    m_dataResource(dataResource),
    m_lastInputResource(lastInputResource),
    m_lastSampleResource(lastSampleResource),
    m_accumulator(accumulator)
{}


void Tracker::track(float value)
{
    // ScopeProfiler profiler(m_title);
    try
    {
        if(!isfinite(value))
            value = 0.0f;

        time_t now = m_clock.now();
        time_t lastInputTimestamp = getTimestamp(m_lastInputResource);
        time_t secondsSinceLastInput = now - lastInputTimestamp;

        if(secondsSinceLastInput <= 0)
            return;

        m_lastInputResource->serialize(now);
        m_accumulator.add(value, secondsSinceLastInput);

        time_t lastSampleTimestamp = getTimestamp(m_lastSampleResource);
        uint32_t timesElapsed = (now - lastSampleTimestamp) / (m_duration_s / m_sampleCount);

        if(timesElapsed > 0)
        {
            // Avoid extremely many newValues after long power off period
            if (timesElapsed > m_sampleCount)
                timesElapsed = m_sampleCount;

            std::vector<float> newValues(timesElapsed - 1, NAN); // Fill values with null while PowerMeter was off
            newValues.push_back(m_accumulator.getAverage());
            updateData(newValues);
            m_accumulator.reset();
        }
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to track " << value;
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


json Tracker::getData() const
{
    try
    {
        json data;
        data["title"] = m_title;
        data["sampleCount"] = m_sampleCount;
        data["duration_s"] = m_duration_s;
        data["data"] = m_dataResource->deserializeOr(json());
        return data;
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to get Data";
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


void Tracker::setData(const json& data)
{
    try
    {
        m_dataResource->serialize(data.at("data"));
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to set Data";
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


void Tracker::erase()
{
    m_dataResource->erase();
    m_lastInputResource->erase();
    m_lastSampleResource->erase();
    m_accumulator.erase();
}


void Tracker::updateData(const std::vector<float>& newValues)
{
    try
    {
        json values = m_dataResource->deserializeOr(json::array_t());

        for (const auto& value : newValues)
            values.push_back(value);

        if(values.size() > m_sampleCount)
            values.erase(values.begin(), values.begin() + values.size() - m_sampleCount);

        if (values.size() > m_sampleCount)
        {
            throw std::runtime_error(SOURCE_LOCATION + "Too many values");
        }

        m_dataResource->serialize(values);
        m_lastSampleResource->serialize(m_clock.now());
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to update Data with " << json(newValues);
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


time_t Tracker::getTimestamp(std::shared_ptr<JsonResource> timestampResource) const
{
    try
    {
        return timestampResource->deserialize();
    }
    catch(...)
    {
        ExceptionTrace::clear();
        timestampResource->serialize(m_clock.now());
        return m_clock.now();
    }
}
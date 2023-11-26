#include "Tracker.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"

#include <math.h>
#include <sstream>

using PM::Tracker;

Tracker::Tracker(
    const std::string& title,
    time_t duration_s,
    size_t sampleCount,
    const Clock& clock,
    const JsonResource& dataResource,
    const JsonResource& lastInputResource,
    const JsonResource& lastSampleResource,
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
    try
    {
        if(!isfinite(value))
            value = 0.0f;

        time_t lastInputTimestamp = getTimestamp(m_lastInputResource);
        time_t secondsSinceLastInput = m_clock.now() - lastInputTimestamp;

        if(secondsSinceLastInput <= 0)
            return;
        
        for(size_t i = 0; i < secondsSinceLastInput; i++)
        {
            m_lastInputResource.serialize(m_clock.now());
            m_accumulator.add(value);
        }
        
        time_t lastSampleTimestamp = getTimestamp(m_lastSampleResource);
        uint32_t timesElapsed = (m_clock.now() - lastSampleTimestamp) / (m_duration_s / m_sampleCount);

        if(timesElapsed > 0)
        {
            for(size_t i = 0; i < timesElapsed - 1; i++)
                updateData(0.0f);

            updateData(m_accumulator.getAverage());
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
        try
        {
            data["data"] = m_dataResource.deserialize();
        }
        catch(...)
        {
            ExceptionTrace::clear();
            data["data"] = json::array_t();
        }
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
        m_dataResource.serialize(data.at("data"));
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
    m_dataResource.erase();
    m_lastInputResource.erase();
    m_lastSampleResource.erase();
    m_accumulator.erase();
}


void Tracker::updateData(float value)
{
    try
    {
        json values;
        try
        {
            values = m_dataResource.deserialize();
        }                     
        catch(...)
        {
            ExceptionTrace::clear();
        }
        values.push_back(value);

        if(values.size() > m_sampleCount)
            values.erase(values.begin());
        
        m_dataResource.serialize(values);
        m_lastSampleResource.serialize(m_clock.now());
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to update Data " << value;
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


time_t Tracker::getTimestamp(const JsonResource& timestampResource) const
{
    try
    {
        return timestampResource.deserialize();
    }
    catch(...) 
    {
        ExceptionTrace::clear();
        timestampResource.serialize(m_clock.now());
        return m_clock.now();
    }
}
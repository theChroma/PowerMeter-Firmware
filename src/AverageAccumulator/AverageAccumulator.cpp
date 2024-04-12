#include "AverageAccumulator.h"
#include "ScopeProfiler/ScopeProfiler.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"


AverageAccumulator::AverageAccumulator(std::shared_ptr<JsonResource> storageResource) :
    m_storageResource(storageResource)
{}


float AverageAccumulator::add(float value, size_t count)
{
    try
    {
        Values values = deserialize();
        values.sum += value * count;
        values.count += count;
        serialize(values);
        return calculateAverage(values);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to add to AverageAccumulator");
        throw;
    }
}


float AverageAccumulator::getAverage() const noexcept
{
    Values values = deserialize();
    return calculateAverage(values);
}


size_t AverageAccumulator::getCount() const noexcept
{
    return deserialize().count;
}


void AverageAccumulator::reset()
{
    serialize(Values(0, 0.0f));
}


void AverageAccumulator::erase()
{
    m_storageResource->erase();
}


void AverageAccumulator::serialize(const Values& values)
{
    json data;
    data["count"] = values.count;
    data["sum"] = values.sum;
    m_storageResource->serialize(data);
}


AverageAccumulator::Values AverageAccumulator::deserialize() const noexcept
{
    try
    {
        json data = m_storageResource->deserialize();
        return Values(data.at("count"), data.at("sum"));
    }
    catch (...)
    {
        return Values(0, 0.0f);
    }
}


float AverageAccumulator::calculateAverage(const Values &values) const noexcept
{
    if(values.count == 0)
        return 0.0f;
    return values.sum / values.count;
}
#pragma once

#include "JsonResource/JsonResource.h"
#include <memory>

class AverageAccumulator
{
public:
    AverageAccumulator(std::unique_ptr<JsonResource> storageResource);
    float getAverage() const noexcept;
    size_t getCount() const noexcept;
    float add(float value, size_t count = 1);
    void reset();
    void remove();

private:
    struct Values
    {
        Values(size_t count, float sum) : count(count), sum(sum) {}
        size_t count;
        float sum;
    };

    void serialize(const Values& values);
    Values deserialize() const noexcept;
    float calculateAverage(const Values& values) const noexcept;

    std::unique_ptr<JsonResource> m_storageResource;
};
#pragma once

#include "JsonResource/JsonResource.h"

namespace PM
{
    class AverageAccumulator
    {
    public:
        AverageAccumulator(const JsonResource& storageResource);
        float getAverage();
        size_t getCount();
        float add(float value);
        void reset();
        void erase() const;

    private:
        void deserialize();
        void serialize();

        size_t m_count = 0;
        float m_sum = 0.0f;
        JsonResource m_storageResource;
    };
}
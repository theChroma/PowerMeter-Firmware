#pragma once

#include <json.hpp>

namespace PM
{
    class Measurement
    {
    public:
        virtual float getTrackerValue() const = 0; 
        virtual json toJson() const = 0; 
        virtual ~Measurement() noexcept {};
    };
}
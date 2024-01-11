#pragma once

#include "Measurement/Measurement.h"
#include <EmonLib.h>
#include <unordered_map>

namespace PM
{
    class MeasuringUnit
    {
    public:
        virtual Measurement& measure() noexcept = 0;
        inline virtual ~MeasuringUnit() noexcept {};
    };
}
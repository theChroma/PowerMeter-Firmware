#pragma once

#include "Measurement/Measurement.h"


class MeasuringUnit
{
public:
    virtual MeasurementList measure() noexcept = 0;
    inline virtual ~MeasuringUnit() noexcept = default;
};
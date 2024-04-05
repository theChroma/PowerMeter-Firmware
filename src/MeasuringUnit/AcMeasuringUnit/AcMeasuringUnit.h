#pragma once

#include "MeasuringUnit/MeasuringUnit.h"
#include <json.hpp>
#include <EmonLib.h>

namespace PM
{
    class AcMeasuringUnit : public MeasuringUnit
    {
    public:
        AcMeasuringUnit(const json& configJson);

        MeasurementList measure() noexcept;

    private:
        EnergyMonitor m_emon;
    };
}
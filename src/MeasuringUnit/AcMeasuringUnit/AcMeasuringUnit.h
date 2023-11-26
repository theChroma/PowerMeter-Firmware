#pragma once

#include "MeasuringUnit/MeasuringUnit.h"
#include "Measurement/AcMeasurement/AcMeasurement.h"
#include <json.hpp>
#include <stdint.h>
#include <EmonLib.h>

namespace PM
{
    class AcMeasuringUnit : public MeasuringUnit
    {
    public:
        AcMeasuringUnit(const json& configJson);

        Measurement& measure() noexcept;
        
    private:
        EnergyMonitor m_emon;
    };
}
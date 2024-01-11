#ifdef ESP32

#include "SimulationMeasuringUnit.h"
#include "Measurement/AcMeasurement/AcMeasurement.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <Arduino.h>
#include <cmath>
#include <cstdlib>
#include <tl/optional.hpp>

using namespace PM;

namespace
{
    float randomInRange(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
}


SimulationMeasuringUnit::SimulationMeasuringUnit(const json &configJson)
{
    try
    {
        m_minVoltage = configJson.at("/voltage/min"_json_pointer);
        m_maxVoltage = configJson.at("/voltage/max"_json_pointer);
        m_minCurrent = configJson.at("/current/min"_json_pointer);
        m_maxCurrent = configJson.at("/current/max"_json_pointer);
        m_minPowerFactor = configJson.at("/powerFactor/min"_json_pointer);
        m_maxPowerFactor = configJson.at("/powerFactor/max"_json_pointer);
        m_measuringRunTime_ms = configJson.at("measuringRunTime_ms");
        Logger[LogLevel::Info] << "Configured simulation measuring unit sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configures simulated measuring unit");
        throw;
    }
}


Measurement& SimulationMeasuringUnit::measure() noexcept
{
    delayMicroseconds(m_measuringRunTime_ms * 1000);
    float simulatedVoltage = randomInRange(m_minVoltage, m_maxVoltage);
    float simulatedCurrent = randomInRange(m_minCurrent, m_maxCurrent);
    float simulatedPowerFactor = randomInRange(m_minPowerFactor, m_maxPowerFactor);
    float simulatedActivePower = simulatedVoltage * simulatedCurrent * simulatedPowerFactor;
    static tl::optional<AcMeasurement> measurement;
    measurement.emplace(simulatedVoltage, simulatedCurrent, simulatedActivePower);
    return measurement.value();
}

#endif
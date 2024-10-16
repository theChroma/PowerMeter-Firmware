#pragma once

#include "MeasuringUnit/MeasuringUnit.h"
#include <json.hpp>

class SimulationMeasuringUnit : public MeasuringUnit
{
public:
    SimulationMeasuringUnit(const json& configJson);

    MeasurementList measure() noexcept override;

private:
    float m_minVoltage;
    float m_maxVoltage;
    float m_minCurrent;
    float m_maxCurrent;
    float m_minPowerFactor;
    float m_maxPowerFactor;
    uint32_t m_measuringRunTime_ms;
};
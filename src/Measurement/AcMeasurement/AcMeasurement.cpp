#include "AcMeasurement.h"
#include <math.h>

using PM::AcMeasurement;

namespace
{
    json createMeasurementJson(const std::string& label, float value, const std::string& unit)
    {
        return {
            {"label", label},
            {"value", value},
            {"unit", unit},
        };
    }
}

AcMeasurement::AcMeasurement(float voltage_V, float current_A, float activePower_W) noexcept :
    m_voltage_V(voltage_V),
    m_current_A(current_A),
    m_activePower_W(activePower_W)
{}


json AcMeasurement::toJson() const noexcept
{
    return {
        createMeasurementJson("Active Power", getActivePower_W(), "W"),
        createMeasurementJson("Apparent Power", getApparentPower_VA(), "VA"),
        createMeasurementJson("Reactive Power", getReactivePower_var(), "var"),
        createMeasurementJson("Power Factor", getPowerFactor(), ""),
        createMeasurementJson("Voltage", getVoltage_V(), "V"),
        createMeasurementJson("Current", getCurrent_A(), "A"),
    };
}

float AcMeasurement::getTrackerValue() const noexcept
{
    return getActivePower_W();
}

float AcMeasurement::getVoltage_V() const noexcept
{
    return m_voltage_V;
}


float AcMeasurement::getCurrent_A() const noexcept
{
    return m_current_A;
}


float AcMeasurement::getActivePower_W() const noexcept
{
    return m_activePower_W;
}


float AcMeasurement::getReactivePower_var() const noexcept
{
    float apparentPower_VA = getApparentPower_VA();
    float reactivePower_var = sqrt(apparentPower_VA * apparentPower_VA - m_activePower_W * m_activePower_W);
    if(isnanf(reactivePower_var))
        return 0.0f;
    return reactivePower_var;
}


float AcMeasurement::getApparentPower_VA() const noexcept
{
    return m_voltage_V * m_current_A;
}


float AcMeasurement::getPowerFactor() const noexcept
{
    float powerFactor = m_activePower_W / getApparentPower_VA();
    if(powerFactor > 0.99f)
        powerFactor = 1.0f;
    if(isnanf(powerFactor))
        powerFactor = 0.0f;
    return powerFactor;
}
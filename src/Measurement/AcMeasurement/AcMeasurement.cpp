#include "AcMeasurement.h"
#include <math.h>

using PM::AcMeasurement;

AcMeasurement::AcMeasurement(float voltage_V, float current_A, float activePower_W) noexcept : 
    m_voltage_V(voltage_V),
    m_current_A(current_A),
    m_activePower_W(activePower_W)
{}


json AcMeasurement::toJson() const noexcept
{
    json measurementJson;
    measurementJson["voltage_V"] = getVoltage_V();
    measurementJson["current_A"] = getCurrent_A();
    measurementJson["activePower_W"] = getActivePower_W();
    measurementJson["apparentPower_VA"] = getApparentPower_VA();
    measurementJson["reactivePower_var"] = getReactivePower_var();
    measurementJson["powerFactor"] = getPowerFactor();
    return measurementJson;
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
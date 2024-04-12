#include "AcPower.h"
#include <math.h>


AcPower::AcPower(float voltage_V, float current_A, float activePower_W) noexcept :
    m_voltage_V(voltage_V),
    m_current_A(current_A),
    m_activePower_W(activePower_W)
{}


float AcPower::getVoltage_V() const noexcept
{
    return m_voltage_V;
}


float AcPower::getCurrent_A() const noexcept
{
    return m_current_A;
}


float AcPower::getActivePower_W() const noexcept
{
    return m_activePower_W;
}


float AcPower::getReactivePower_var() const noexcept
{
    float apparentPower_VA = getApparentPower_VA();
    float reactivePower_var = sqrt(apparentPower_VA * apparentPower_VA - m_activePower_W * m_activePower_W);
    if(!isfinite(reactivePower_var))
        return 0.0f;
    return reactivePower_var;
}


float AcPower::getApparentPower_VA() const noexcept
{
    return m_voltage_V * m_current_A;
}


float AcPower::getPowerFactor() const noexcept
{
    float powerFactor = m_activePower_W / getApparentPower_VA();
    if(powerFactor > 0.99f)
        powerFactor = 1.0f;
    if(!isfinite(powerFactor))
        powerFactor = 0.0f;
    return powerFactor;
}
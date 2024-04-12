#pragma once

class AcPower
{
public:
    AcPower(float voltage_V, float current_A, float activePower_W) noexcept;
    float getVoltage_V() const noexcept;
    float getCurrent_A() const noexcept;
    float getActivePower_W() const noexcept;
    float getReactivePower_var() const noexcept;
    float getApparentPower_VA() const noexcept;
    float getPowerFactor() const noexcept;

private:
    float m_voltage_V;
    float m_current_A;
    float m_activePower_W;
};

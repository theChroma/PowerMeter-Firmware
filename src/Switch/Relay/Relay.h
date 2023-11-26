#pragma once

#include "Switch/Switch.h"
#include <json.hpp>

namespace PM
{
    class Relay : public Switch
    {
    public:
        Relay(const json& configJson);

        tl::optional<bool> getState() const noexcept override;
        void setState(bool state) override;
    
    private:
        uint8_t m_pin;
        bool m_isNormallyOpen;
    };
}
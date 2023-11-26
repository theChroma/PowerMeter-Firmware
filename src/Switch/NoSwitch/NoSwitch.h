#pragma once

#include "Switch/Switch.h"
#include <json.hpp>

namespace PM
{
    class NoSwitch : public Switch
    {
    public:
        NoSwitch(const json& configJson) noexcept;

        tl::optional<bool> getState() const noexcept override;
        inline void setState(bool state) noexcept override
        {}
    };
}
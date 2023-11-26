#pragma once

#include <tl/optional.hpp>

namespace PM
{
    class Switch
    {
    public:
        virtual tl::optional<bool> getState() const = 0;
        virtual void setState(bool state) = 0;
        inline virtual ~Switch() noexcept {};
    };
}
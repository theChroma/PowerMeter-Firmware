#pragma once

#include <time.h>

namespace PM
{
    class Clock
    {
    public:
        virtual time_t now() const noexcept = 0;
        inline virtual ~Clock() noexcept {};
    };
}
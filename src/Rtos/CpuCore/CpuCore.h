#pragma once

#include <string>

namespace PM
{
    namespace Rtos
    {
        struct CpuCore
        {
            enum Value : int
            {
                Auto = -1,
                Core0 = 0,
                Core1 = 1,
            };

            constexpr CpuCore(Value value) : value(value) {}
            constexpr operator Value() const { return value; }
            explicit operator bool() const = delete;

            const char* getResetReason() const;

            Value value;
        };

    }
}
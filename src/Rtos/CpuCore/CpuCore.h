#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>

namespace Rtos
{
    struct CpuCore
    {
        enum Value : int
        {
            Auto = tskNO_AFFINITY,
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
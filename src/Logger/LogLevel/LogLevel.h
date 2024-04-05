#pragma once

#include <string>

namespace PM
{
    struct LogLevel
    {
        enum Value
        {
            Error = 0,
            Warning = 1,
            Info = 2,
            Debug = 3,
            Verbose = 4,
        };

        constexpr LogLevel(Value value) noexcept : value(value) {}
        LogLevel(std::string name);
        constexpr operator Value() const noexcept { return value; }
        explicit operator bool() const = delete;
        operator std::string() const noexcept;

        Value value;
    };

    std::ostream& operator<<(std::ostream& os, const LogLevel& level) noexcept;
}
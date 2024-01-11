#pragma once

#include <string>
#include <deque>

namespace PM
{
    namespace ExceptionTrace
    {
        using Traces = std::deque<std::string>;

        void trace(const std::string& message) noexcept;
        void clear() noexcept;
        Traces get(bool clearTraces = true) noexcept;
        std::string what(bool clearTraces = true, size_t indentLevel = 1, char indentChar = ' ') noexcept;
    }
}
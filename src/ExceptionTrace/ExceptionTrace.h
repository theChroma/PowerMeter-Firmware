#pragma once

#include <string>

namespace PM
{
    namespace ExceptionTrace
    {
        void trace(const std::string& message) noexcept;
        void clear() noexcept;
        std::string what(size_t indentLevel = 1, char indentChar = ' ');
    }
}
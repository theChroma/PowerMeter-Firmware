#pragma once

#include "Logger/LogLevel/LogLevel.h"
#include <json.hpp>
#include <sstream>
#include <iostream>

namespace PM
{
    class LogStream
    {
    public:
        LogStream(LogLevel minLevel, LogLevel maxLevel, std::ostream& stream, bool showLevel) noexcept;
        std::ostream& operator[](LogLevel level) noexcept;

    private:
        LogLevel m_minLevel;
        LogLevel m_maxLevel;
        bool m_showLevel;
        std::ostream& m_stream;
    };
}
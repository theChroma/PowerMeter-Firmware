#pragma once

#include "Logger/LogLevel/LogLevel.h"
#include "Logger/LogStream/LogStream.h"
#include <iostream>
#include <vector>

namespace PM
{
    class MultiLogger
    {
	public:
		MultiLogger(const std::vector<LogStream>& streams);
        std::ostream& operator[](LogLevel level) noexcept;

	private:
        std::vector<LogStream> m_streams;
    };
}
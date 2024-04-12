#include "ScopeProfiler.h"
#include <Arduino.h>
#include "Logger/Logger.h"


ScopeProfiler::ScopeProfiler(const std::string& name) : m_name(name)
{
    m_profileStartTime_us = micros();
}

ScopeProfiler::~ScopeProfiler()
{
    uint32_t profileDuration_us = micros() - m_profileStartTime_us;
    Logger[LogLevel::Debug]
        << '"'
        << m_name
        << "\" took "
        << profileDuration_us
        << " µs, or "
        << profileDuration_us / 1000
        << " ms" << std::endl;
}
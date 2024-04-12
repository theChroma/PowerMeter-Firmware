#ifdef ESP32

#include "SimulationClock.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "Arduino.h"


SimulationClock::SimulationClock(const json &configJson)
{
    try
    {
        m_startTimestamp = configJson.at("startTimestamp");
        m_fastForward = configJson.at("fastForward");
        Logger[LogLevel::Info] << "Configured simulated clock sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure simulated clock");
        throw;
    }
}


time_t SimulationClock::now() const noexcept
{
    return m_startTimestamp + millis() * 0.001f * m_fastForward;
}

#endif
#ifdef ESP32

#include "DS3231.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <exception>

using PM::DS3231;


DS3231::DS3231(const json &configJson)
{
    try
    {
        if(!m_rtc.begin())
            throw std::runtime_error("Failed to begin I2C Communication to DS3231");
        Logger[LogLevel::Info] << "Configured DS3231 clock sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure DS3231");
        throw;
    }
}


time_t DS3231::now() const noexcept
{
    RTC_DS3231 rtc = m_rtc;
    return rtc.now().unixtime();
}

#endif
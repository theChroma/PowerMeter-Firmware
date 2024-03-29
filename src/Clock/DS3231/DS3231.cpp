#ifdef ESP32

#include "DS3231.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <exception>
#include <sys/time.h>

using PM::DS3231;


DS3231::DS3231(const json &configJson)
{
    try
    {
        if(!m_rtc.begin())
            throw std::runtime_error(SOURCE_LOCATION + "Failed to begin I2C Communication to DS3231");

        timeval systemTime = {
            .tv_sec = now(),
            .tv_usec = 0,
        };
        timezone systemTimezone = {
            .tz_minuteswest = 0,
            .tz_dsttime = 0,
        };
        if (settimeofday(&systemTime, &systemTimezone) != 0)
        {
            Logger[LogLevel::Warning] << "Failed to sync systemtime with DS3231." << std::endl;
        }
        time_t sysNow;
        time(&sysNow);
        Logger[LogLevel::Debug] << "Now: " << sysNow << std::endl;
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
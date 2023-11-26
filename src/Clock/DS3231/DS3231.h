#pragma once

#include "Clock/Clock.h"
#include <RTClib.h>
#include <json.hpp>

namespace PM
{
    class DS3231 : public Clock
    {
    public:
        DS3231(const json& configJson);
        
        time_t now() const noexcept override;

    private:
        RTC_DS3231 m_rtc;
    };
}
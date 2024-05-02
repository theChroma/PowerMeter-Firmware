#ifdef ESP32

#include "CpuCore.h"
#include <esp32/rom/rtc.h>

using namespace Rtos;

const char* CpuCore::getResetReason() const
{
    switch (rtc_get_reset_reason(value))
    {
        case 1:  return "POWERON_RESET";
        case 3:  return "SW_RESET";
        case 4:  return "OWDT_RESET";
        case 5:  return "DEEPSLEEP_RESET";
        case 6:  return "SDIO_RESET";
        case 7:  return "TG0WDT_SYS_RESET";
        case 8:  return "TG1WDT_SYS_RESET";
        case 9:  return "RTCWDT_SYS_RESET";
        case 10: return "INTRUSION_RESET";
        case 11: return "TGWDT_CPU_RESET";
        case 12: return "SW_CPU_RESET";
        case 13: return "RTCWDT_CPU_RESET";
        case 14: return "EXT_CPU_RESET";
        case 15: return "RTCWDT_BROWN_OUT_RESET";
        case 16: return "RTCWDT_RTC_RESET";
    }
    return "Unknown";
}

#endif
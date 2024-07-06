#ifdef ESP32

#include "AcMeasuringUnit.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "AcPower/AcPower.h"
#include <EmonLib.h>
#include <algorithm>
#include <math.h>


AcMeasuringUnit::AcMeasuringUnit(const json& configJson)
{
    try
    {
        m_emon.voltage(
            configJson.at("/pins/voltage"_json_pointer),
            configJson.at("/calibration/voltage"_json_pointer),
            configJson.at("/calibration/phase"_json_pointer)
        );
        m_emon.current(
            configJson.at("/pins/current"_json_pointer),
            configJson.at("/calibration/current"_json_pointer)
        );
        Logger[LogLevel::Info] << "Configured AC measuring unit sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure AC measuring unit");
        throw;
    }
}


MeasurementList AcMeasuringUnit::measure() noexcept
{
    m_emon.calcVI(40, 4000);
    AcPower acPower(m_emon.Vrms, m_emon.Irms, m_emon.realPower);
    return MeasurementList {
        Measurement {
            .name = "Active Power",
            .value = acPower.getActivePower_W(),
            .unit = "W",
            .fractionDigits = 0,
        },
        Measurement {
            .name = "Apparent Power",
            .value = acPower.getApparentPower_VA(),
            .unit = "VA",
            .fractionDigits = 0,
        },
        Measurement {
            .name = "Reactive Power",
            .value = acPower.getReactivePower_var(),
            .unit = "var",
            .fractionDigits = 0,
        },
        Measurement {
            .name = "Voltage",
            .value = acPower.getVoltage_V(),
            .unit = "V",
            .fractionDigits = 0,
        },
        Measurement {
            .name = "Current",
            .value = acPower.getCurrent_A(),
            .unit = "A",
            .fractionDigits = 1,
        },
        Measurement {
            .name = "Power Factor",
            .value = acPower.getPowerFactor(),
            .unit = "",
            .fractionDigits = 2,
        },
    };
}

#endif
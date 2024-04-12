#ifdef ESP32

#include "AcMeasuringUnit.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "AcPower/AcPower.h"
#include <EmonLib.h>
#include <algorithm>
#include <math.h>


namespace
{
    float calculateReactivePower(float activePower, float apparentPower)
    {
        float reactivePower = sqrt(apparentPower * apparentPower - activePower * activePower);
        if(!isfinite(reactivePower))
            return 0.0f;
        return reactivePower;
    }

    float clamp(float value, float lowerLimit, float upperLimit) {
        return std::max(lowerLimit, std::min(value, upperLimit));
    }
}


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
        },
        Measurement {
            .name = "Apparent Power",
            .value = acPower.getApparentPower_VA(),
            .unit = "VA",
        },
        Measurement {
            .name = "Reactive Power",
            .value = acPower.getReactivePower_var(),
            .unit = "var",
        },
        Measurement {
            .name = "Voltage",
            .value = acPower.getVoltage_V(),
            .unit = "V",
        },
        Measurement {
            .name = "Current",
            .value = acPower.getCurrent_A(),
            .unit = "A",
        },
        Measurement {
            .name = "Power Factor",
            .value = acPower.getPowerFactor(),
            .unit = "",
        },
    };
}

#endif
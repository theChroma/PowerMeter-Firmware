#ifdef ESP32

#include "AcMeasuringUnit.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <EmonLib.h>
#include <tl/optional.hpp>
#include <math.h>


using namespace PM;


AcMeasuringUnit::AcMeasuringUnit(const json& configJson)
{
    try
    {
        m_emon.voltage(
            configJson.at("/pins/voltage"_json_pointer),
            configJson.at("/pins/current"_json_pointer),
            configJson.at("/calibration/voltage"_json_pointer)
        );
        m_emon.current(
            configJson.at("/calibration/current"_json_pointer),
            configJson.at("/calibration/phase"_json_pointer)
        );
        Logger[LogLevel::Info] << "Configured emon measuring unit sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Emon measuring unit");
        throw;
    }
}


Measurement& AcMeasuringUnit::measure() noexcept
{
    m_emon.calcVI(40, 4000);
    static tl::optional<AcMeasurement> measurement;
    measurement.emplace(m_emon.Vrms, m_emon.Irms, m_emon.realPower);
    return measurement.value();
}

#endif
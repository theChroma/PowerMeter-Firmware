#if defined(ESP32) && !defined(PIO_UNIT_TESTING)

#include "Api/Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "HTTPServer/HTTPServer.h"
#include "RestAPI/RestAPI.h"
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <tl/optional.hpp>

using namespace PM;

namespace
{
    struct MeasuringContext
    {
        MeasuringUnit& measuringUnit;
        Measurement& measurement;
        TrackerMap& trackers;
    };
    
    const JsonResource loggerConfigResource("/Config/Logger.json");
    const JsonResource wifiConfigResource("/Config/Wifi.json");
    const JsonResource measuringConfigResource("/Config/Measuring.json");
    const JsonResource switchConfigResource("/Config/Switch.json");
    const JsonResource clockConfigResource("/Config/Clock.json");
    const JsonResource trackerConfigResource("/Config/Trackers.json");
}

void measure(void* context)
{
    try
    {
        while (true)
        {
            MeasuringContext& measuringContext = *static_cast<MeasuringContext*>(context);
            
            measuringContext.measurement = measuringContext.measuringUnit.measure();

            for (auto& tracker : measuringContext.trackers)
                tracker.second.track(measuringContext.measurement.getTrackerValue());

            delay(500);
        }
    }
    catch(...)
    {
        Logger[LogLevel::Error] 
            << SOURCE_LOCATION << "An Exception occurred, here is what happened:\n"
            << ExceptionTrace::what() << std::endl;
        vTaskDelete(NULL);
    }
}

void setup()
{
    Serial.begin(115200);
    LittleFS.begin(true, "", 30);

    try
    {
        
        Config::configureLogger(loggerConfigResource);
        Logger[LogLevel::Info] << "Booting..." << std::endl;
        
        Config::configureWiFi(wifiConfigResource);
        
        static HTTPServer server(80, {{"Access-Control-Allow-Origin", "*"}});
        server.start();
        static RestAPI api(server, "/api");
        
        static MeasuringUnit& measuringUnit = Config::configureMeasuringUnit(measuringConfigResource);
        static Switch& switchUnit = Config::configureSwitch(switchConfigResource);
        static Clock& clock = Config::configureClock(clockConfigResource);
        static TrackerMap trackers = Config::configureTrackers(trackerConfigResource, clock);

        Api::createSystemEndpoints(api);
        Api::createLoggerEndpoints(api, loggerConfigResource);
        static Measurement& measurement = measuringUnit.measure();
        Api::createMeasuringEndpoints(api, measuringConfigResource, measuringUnit, measurement);
        Api::createSwitchEndpoints(api, switchConfigResource, switchUnit);
        Api::createTrackerEndpoints(api, trackerConfigResource, trackers, clock);
        Api::createWiFiEndpoints(api, wifiConfigResource);

        static MeasuringContext measuringContext = {
            .measuringUnit = measuringUnit,
            .measurement = measurement,
            .trackers = trackers,
        };
        xTaskCreatePinnedToCore(measure, "measuring", 8000, &measuringContext, 10, nullptr, 1);
        Logger[LogLevel::Info] << "Boot sequence finished. Running..." << std::endl;
    }
    catch(...)
    {
        Logger[LogLevel::Error] 
            << SOURCE_LOCATION << "An Exception occurred, here is what happened:\n"
            << ExceptionTrace::what() << std::endl;
    }
}

void loop()
{
    vTaskDelete(NULL);
}

#endif
#if defined(ESP32) && !defined(PIO_UNIT_TESTING)

#include "Api/Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "RestAPI/RestAPI.h"
#include <tuple>
#include <LittleFS.h>
#include <ElegantOTA.h>

using namespace PM;

namespace
{
    struct MeasuringContext
    {
        std::reference_wrapper<MeasuringUnit> measuringUnit;
        std::reference_wrapper<Measurement> measurement;
        TrackerMap trackers;
    };
    
    const Version firmwareVersion(
        POWERMETER_FIRMWARE_VERSION_MAJOR,
        POWERMETER_FIRMWARE_VERSION_MINOR,
        POWERMETER_FIRMWARE_VERSION_PATCH
    );
    const Version apiVersion(
        POWERMETER_API_VERSION_MAJOR,
        POWERMETER_API_VERSION_MINOR,
        POWERMETER_API_VERSION_PATCH
    );
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
        MeasuringContext& measuringContext = *static_cast<MeasuringContext*>(context);        
        measuringContext.measurement = measuringContext.measuringUnit.get().measure();

        for (auto& tracker : measuringContext.trackers)
            tracker.second.track(measuringContext.measurement.get().getTrackerValue());

        delay(500);
    }
    catch(...)
    {
        Logger[LogLevel::Error] 
            << "Exception occurred at " << SOURCE_LOCATION << "\r\n"
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
        static AsyncWebServer server(80);

        static RestAPI api(
            server,
            apiVersion,
            "/api"
        );
        ElegantOTA.begin(&server);
        server.serveStatic("/", LittleFS, "/App/").setDefaultFile("index.html");
        server.onNotFound([](AsyncWebServerRequest *request){
            request->send(404, "text/plain", "Not Found");
        });
        
        Config::configureLogger(loggerConfigResource, server);
        Logger[LogLevel::Info] << "Booting..." << std::endl;
        Logger[LogLevel::Info] << "Firmware version v" << firmwareVersion << std::endl;
        Logger[LogLevel::Info] << "API version v" << apiVersion << std::endl;
        Config::configureWiFi(wifiConfigResource);
        server.begin();
        static std::reference_wrapper<Switch> switchUnit = Config::configureSwitch(switchConfigResource);
        static std::reference_wrapper<Clock> clock = Config::configureClock(clockConfigResource);
        static MeasuringContext measuringContext = {
            .measuringUnit = Config::configureMeasuringUnit(measuringConfigResource),
            .measurement = measuringContext.measuringUnit.get().measure(),
            .trackers = Config::configureTrackers(trackerConfigResource, clock),
        };

        Api::createSystemEndpoints(api);
        Api::createLoggerEndpoints(api, loggerConfigResource, server);
        Api::createSwitchEndpoints(api, switchConfigResource, switchUnit);
        Api::createClockEndpoints(api, clockConfigResource, clock);
        Api::createTrackerEndpoints(api, trackerConfigResource, measuringContext.trackers, clock);
        Api::createWiFiEndpoints(api, wifiConfigResource);
        Api::createMeasuringEndpoints(
            api,
            measuringConfigResource,
            measuringContext.measuringUnit,
            measuringContext.measurement
        );

        Logger[LogLevel::Info] << "Boot sequence finished. Running..." << std::endl;
        
        xTaskCreateUniversal(
            // Task function must be wrapped, to allow exception handling
            [](void* context){ while (true) measure(context); },
            "measuring",
            8000,
            &measuringContext,
            10,
            nullptr,
            1
        );
    }
    catch(...)
    {
        Logger[LogLevel::Error] 
            << "Exception occurred at " << SOURCE_LOCATION << "\r\n"
            << ExceptionTrace::what() << std::endl;
    }
}

void loop()
{
    vTaskDelete(nullptr);
}

#endif
#if defined(ESP32) && !defined(PIO_UNIT_TESTING)

#include "Api/Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "FileBrowser/FileBrowser.h"
#include "RestAPI/RestAPI.h"
#include "esp32/rom/rtc.h"
#include <tuple>
#include <LittleFS.h>
#include <ElegantOTA.h>
#include <fstream>

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
    const JsonResource networkConfigResource("/Config/Network.json");
    const JsonResource measuringConfigResource("/Config/Measuring.json");
    const JsonResource switchConfigResource("/Config/Switch.json");
    const JsonResource clockConfigResource("/Config/Clock.json");
    const JsonResource trackerConfigResource("/Config/Trackers.json");

    const char* getResetReason(uint8_t cpu)
    {
        switch (rtc_get_reset_reason(cpu))
        {
            case 1  : return "Vbat power on reset";
            case 3  : return "Software reset digital core";
            case 4  : return "Legacy watch dog reset digital core";
            case 5  : return "Deep Sleep reset digital core";
            case 6  : return "Reset by SLC module, reset digital core";
            case 7  : return "Timer Group0 Watch dog reset digital core";
            case 8  : return "Timer Group1 Watch dog reset digital core";
            case 9  : return "RTC Watch dog Reset digital core";
            case 10 : return "Instrusion tested to reset CPU";
            case 11 : return "Time Group reset CPU";
            case 12 : return "Software reset CPU";
            case 13 : return "RTC Watch dog Reset CPU";
            case 14 : return "for APP CPU, reseted by PRO CPU";
            case 15 : return "Reset when the vdd voltage is not stable";
            case 16 : return "RTC Watch dog reset digital core and rtc module";
        }
        return "NO_MEAN";
    }
}

void measure(void* context)
{
    try
    {
        MeasuringContext& measuringContext = *static_cast<MeasuringContext*>(context);
        measuringContext.measurement = measuringContext.measuringUnit.get().measure();

        // for (auto& tracker : measuringContext.trackers)
        //     tracker.second.track(measuringContext.measurement.get().getTrackerValue());

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
    try
    {
        if (!LittleFS.begin(false, "", 30))
            throw std::runtime_error("Failed to mount Filesystem");



        const JsonResource writeTestResource("/writeTest/writeTest.json");
        try
        {
            writeTestResource.deserialize();
            Logger[LogLevel::Debug] << "Deserialize success!" << std::endl;

            size_t count = 0;
            try
            {

                Logger[LogLevel::Debug] << "Starting write test..." << std::endl;
                for (count = 0; count < 1000; count++)
                {
                    json data = {
                        {"text", "Lorem ipsum dolor sit amet "},
                        {"count", count},
                    };
                    writeTestResource.serialize(data);
                    json readData = writeTestResource.deserialize();
                    if (data != readData)
                    {
                        throw std::runtime_error("Doesn't matcht");
                    }
                }
                Logger[LogLevel::Debug] << "Write test finished!" << std::endl;
            }
            catch(...)
            {
                Logger[LogLevel::Error]
                    << "Exception occurred at " << SOURCE_LOCATION << "\r\n"
                    << ExceptionTrace::what() << std::endl;
                Logger[LogLevel::Debug] << "count="<< count << std::endl;
            }
        }
        catch(...)
        {
            Logger[LogLevel::Debug] << "Deserialize failed" << std::endl;
        }




        static AsyncWebServer server(80);
        static RestAPI restApi(
            server,
            apiVersion,
            "/api"
        );
        ElegantOTA.begin(&server);
        FileBrowser::serve(server, "/files");
        server.serveStatic("/", LittleFS, "/App/").setDefaultFile("index.html");
        server.onNotFound([](AsyncWebServerRequest *request){
            request->send(404, "text/plain", "Not Found");
        });

        Config::configureLogger(loggerConfigResource, server);
        Logger[LogLevel::Debug] << "Reset reason CPU0: " << getResetReason(0) << std::endl;
        Logger[LogLevel::Debug] << "Reset reason CPU1: " << getResetReason(1) << std::endl;
        Logger[LogLevel::Info] << "Booting..." << std::endl;
        Logger[LogLevel::Info] << "Firmware version v" << firmwareVersion << std::endl;
        Logger[LogLevel::Info] << "API version v" << apiVersion << std::endl;
        Config::configureNetwork(networkConfigResource);
        server.begin();
        static std::reference_wrapper<Switch> switchUnit = Config::configureSwitch(switchConfigResource);
        static std::reference_wrapper<Clock> clock = Config::configureClock(clockConfigResource);
        static MeasuringContext measuringContext = {
            .measuringUnit = Config::configureMeasuringUnit(measuringConfigResource),
            .measurement = measuringContext.measuringUnit.get().measure(),
            .trackers = Config::configureTrackers(trackerConfigResource, clock),
        };

        Api api(restApi);
        api.createSystemEndpoints(firmwareVersion, apiVersion);
        api.createLoggerEndpoints(loggerConfigResource, server);
        api.createSwitchEndpoints(switchConfigResource, switchUnit);
        api.createClockEndpoints(clockConfigResource, clock);
        api.createTrackerEndpoints(trackerConfigResource, measuringContext.trackers, clock);
        api.createNetworkEndpoints(networkConfigResource);
        api.createMeasuringEndpoints(
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
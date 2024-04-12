#if defined(ESP32) && !defined(PIO_UNIT_TESTING)

#include "Api/Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"
#include "ScopeProfiler/ScopeProfiler.h"
#include "FileBrowser/FileBrowser.h"
#include "RestApi/RestApi.h"
#include "Rtos/Task/Task.h"
#include "Rtos/ValueMutex/ValueMutex.h"
#include <tuple>
#include <LittleFS.h>
#include <ElegantOTA.h>
#include <fstream>


void setup()
{
    Serial.begin(115200);
    try
    {
        if (!LittleFS.begin(true, "", 30))
            throw std::runtime_error("Failed to mount Filesystem");

        static BackedUpJsonResource switchConfigResource("/Config/Switch.json");
        static BackedUpJsonResource loggerConfigResource("/Config/Logger.json");
        static BackedUpJsonResource networkConfigResource("/Config/Network.json");
        static BackedUpJsonResource measuringConfigResource("/Config/Measuring.json");
        static BackedUpJsonResource clockConfigResource("/Config/Clock.json");
        static BackedUpJsonResource trackerConfigResource("/Config/Trackers.json");

        static const Version firmwareVersion(
            POWERMETER_FIRMWARE_VERSION_MAJOR,
            POWERMETER_FIRMWARE_VERSION_MINOR,
            POWERMETER_FIRMWARE_VERSION_PATCH
        );
        static const Version apiVersion(
            POWERMETER_API_VERSION_MAJOR,
            POWERMETER_API_VERSION_MINOR,
            POWERMETER_API_VERSION_PATCH
        );


        static AsyncWebServer server(80);
        static RestApi restApi(
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
        Logger[LogLevel::Debug] << "Reset reason CPU Core 0: " << Rtos::CpuCore(Rtos::CpuCore::Core0).getResetReason() << std::endl;
        Logger[LogLevel::Debug] << "Reset reason CPU Core 1: " << Rtos::CpuCore(Rtos::CpuCore::Core1).getResetReason() << std::endl;
        Logger[LogLevel::Info] << "Booting..." << std::endl;
        Logger[LogLevel::Info] << "Firmware version v" << firmwareVersion << std::endl;
        Logger[LogLevel::Info] << "API version v" << apiVersion << std::endl;
        Config::configureNetwork(networkConfigResource);
        server.begin();
        static std::reference_wrapper<Switch> switchUnit = Config::configureSwitch(switchConfigResource);
        static std::reference_wrapper<Clock> clock = Config::configureClock(clockConfigResource);
        static std::reference_wrapper<MeasuringUnit> measuringUnit = Config::configureMeasuring(measuringConfigResource);
        static Rtos::ValueMutex<MeasurementList> sharedMeasurements;
        static Rtos::ValueMutex<TrackerMap> sharedTrackers;
        sharedTrackers = Config::configureTrackers(trackerConfigResource, clock);

        Api::createSystemEndpoints(restApi, firmwareVersion, apiVersion);
        Api::createLoggerEndpoints(restApi, loggerConfigResource, server);
        Api::createSwitchEndpoints(restApi, switchConfigResource, switchUnit);
        Api::createClockEndpoints(restApi, clockConfigResource, clock);
        Api::createTrackerEndpoints(restApi, trackerConfigResource, sharedTrackers, clock);
        Api::createNetworkEndpoints(restApi, networkConfigResource);
        Api::createMeasuringEndpoints(restApi, measuringConfigResource, measuringUnit, sharedMeasurements);

        Logger[LogLevel::Info] << "Boot sequence finished. Running..." << std::endl;


        static Rtos::Task measuringTask("Measuring", 10, 3000, [](Rtos::Task& task){
                while (true)
                {
                    sharedMeasurements = measuringUnit.get().measure();
                    delay(1000);
                }
            },
            Rtos::CpuCore::Core1
        );

        static Rtos::Task trackerTask("Tracker", 1, 8000, [](Rtos::Task& task){
            while (true)
            {
                MeasurementList measurements = sharedMeasurements;
                if (measurements.size() == 0)
                    continue;
                TrackerMap trackers = sharedTrackers;
                for (auto& tracker : trackers)
                    tracker.second.track(measurements.front().value);
                delay(1000);
            }
        });
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
    Rtos::Task::getCurrent().cancel();
}

#endif
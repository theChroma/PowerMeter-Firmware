#if defined(ESP32) && !defined(PIO_UNIT_TESTING)

#include "Api/Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"
#include "JsonResource/BasicJsonResource/BasicJsonResource.h"
#include "ScopeProfiler/ScopeProfiler.h"
#include "FileBrowser/FileBrowser.h"
#include "Filesystem/Directory/LittleFsDirectory/LittleFsDirectory.h"
#include "Filesystem/File/LittleFsFile/LittleFsFile.h"
#include "RestApi/RestApi.h"
#include "Rtos/Task/Task.h"
#include "Rtos/ValueMutex/ValueMutex.h"
#include "WifiScan/WifiScan.h"
#include <tuple>
#include <LittleFS.h>
#include <ElegantOTA.h>
#include <fstream>
#include <esp_task_wdt.h>


void setup()
{
    Serial.begin(115200);
    try
    {
        if (!LittleFS.begin(true, "", 30))
            throw std::runtime_error("Failed to mount Filesystem");

        // static BackedUpJsonResource switchConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Switch.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Switch.b.json")))
        // );
        // static BackedUpJsonResource loggerConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Logger.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Logger.b.json")))
        // );
        // static BackedUpJsonResource networkConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Network.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Network.b.json")))
        // );
        // static BackedUpJsonResource measuringConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Measuring.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Measuring.b.json")))
        // );
        // static BackedUpJsonResource clockConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Clock.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Clock.b.json")))
        // );
        // static BackedUpJsonResource trackerConfigResource(
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Tracker.a.json"))),
        //     BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Tracker.b.json")))
        // );

        static BasicJsonResource switchConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Switch.json"))
        );
        static BasicJsonResource loggerConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Logger.json"))
        );
        static BasicJsonResource networkConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Network.json"))
        );
        static BasicJsonResource measuringConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Measuring.json"))
        );
        static BasicJsonResource clockConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Clock.json"))
        );
        static BasicJsonResource trackerConfigResource(
            std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Tracker.json"))
        );

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

        Config::configureLogger(&loggerConfigResource, &server);
        Logger[LogLevel::Debug] << "Reset reason CPU Core 0: " << Rtos::CpuCore(Rtos::CpuCore::Core0).getResetReason() << std::endl;
        Logger[LogLevel::Debug] << "Reset reason CPU Core 1: " << Rtos::CpuCore(Rtos::CpuCore::Core1).getResetReason() << std::endl;
        Logger[LogLevel::Info] << "Booting..." << std::endl;
        Logger[LogLevel::Info] << "Firmware version v" << firmwareVersion << std::endl;
        Logger[LogLevel::Info] << "API version v" << apiVersion << std::endl;
        Config::configureNetwork(&networkConfigResource);
        server.begin();
        static Switch* switchUnit = Config::configureSwitch(&switchConfigResource);
        static Clock* clock = Config::configureClock(&clockConfigResource);
        static MeasuringUnit* measuringUnit = Config::configureMeasuring(&measuringConfigResource);
        static Rtos::ValueMutex<MeasurementList> measurementsValueMutex;
        static Rtos::ValueMutex<TrackerMap> trackersValueMutex;
        trackersValueMutex = Config::configureTrackers(&trackerConfigResource, clock);

        Api::createSystemEndpoints(&restApi, firmwareVersion, apiVersion);
        Api::createLoggerEndpoints(&restApi, &loggerConfigResource, &server);
        Api::createSwitchEndpoints(&restApi, &switchConfigResource, &switchUnit);
        Api::createClockEndpoints(&restApi, &clockConfigResource, &clock);
        Api::createTrackerEndpoints(&restApi, &trackerConfigResource, &trackersValueMutex, clock);
        Api::createNetworkEndpoints(&restApi, &networkConfigResource);
        Api::createMeasuringEndpoints(&restApi, &measuringConfigResource, &measuringUnit, &measurementsValueMutex);

        Logger[LogLevel::Info] << "Boot sequence finished. Running..." << std::endl;

        static Rtos::Task measuringTask("Measuring", 10, 3000, [](Rtos::Task& task){
                while (true)
                {
                    measurementsValueMutex = measuringUnit->measure();
                    delay(1000);
                }
            },
            Rtos::CpuCore::Core1
        );

        static Rtos::Task trackerTask("Tracker", 1, 8000, [](Rtos::Task& task){
            while (true)
            {
                Rtos::ValueMutex<MeasurementList>::Lock measurements = measurementsValueMutex.get();
                if (measurements->size() > 0)
                {
                    Rtos::ValueMutex<TrackerMap>::Lock trackers = trackersValueMutex.get();
                    for (auto& tracker : *trackers)
                        tracker.second.track(measurements->front().value);
                }
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
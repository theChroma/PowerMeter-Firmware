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
    esp_task_wdt_init(10, true);
    Serial.begin(115200);
    try
    {
        if (!LittleFS.begin(true, "", 30))
            throw std::runtime_error("Failed to mount Filesystem");

        static BackedUpJsonResource switchConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Switch.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Switch.b.json")))
        );
        static BackedUpJsonResource loggerConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Logger.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Logger.b.json")))
        );
        static BackedUpJsonResource networkConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Network.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Network.b.json")))
        );
        static BackedUpJsonResource measuringConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Measuring.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Measuring.b.json")))
        );
        static BackedUpJsonResource clockConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Clock.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Clock.b.json")))
        );
        static BackedUpJsonResource trackerConfigResource(
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Tracker.a.json"))),
            BasicJsonResource(std::unique_ptr<Filesystem::File>(new Filesystem::LittleFsFile("/Config/Tracker.b.json")))
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
        server.serveStatic("/", LittleFS, "/App/").setDefaultFile("index.html");
        server.serveStatic("/", LittleFS, "/");
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
        static Switch* switchUnit = Config::configureSwitch(&switchConfigResource);
        static Clock* clock = Config::configureClock(&clockConfigResource);
        static MeasuringUnit* measuringUnit = Config::configureMeasuring(&measuringConfigResource);
        static Rtos::ValueMutex<MeasurementList> measurementsValueMutex;
        static Rtos::ValueMutex<TrackerMap> trackersValueMutex;
        *trackersValueMutex.get() = Config::configureTrackers(&trackerConfigResource, clock);

        Api::createSystemEndpoints(&restApi, firmwareVersion, apiVersion);
        Api::createLoggerEndpoints(&restApi, &loggerConfigResource, &server);
        Api::createSwitchEndpoints(&restApi, &switchConfigResource, &switchUnit);
        Api::createClockEndpoints(&restApi, &clockConfigResource, &clock);
        Api::createTrackerEndpoints(&restApi, &trackerConfigResource, &trackersValueMutex, clock);
        Api::createNetworkEndpoints(&restApi, &networkConfigResource);
        Api::createMeasuringEndpoints(&restApi, &measuringConfigResource, &measuringUnit, &measurementsValueMutex);
        server.begin();

        Logger[LogLevel::Info] << "Boot sequence finished. Running..." << std::endl;

        static Rtos::Task measuringTask("Measuring", 10, 3000, [](Rtos::Task* task){
                while (true)
                {
                    *measurementsValueMutex.get() = measuringUnit->measure();
                    delay(1000);
                }
            },
            Rtos::CpuCore::Core1
        );

        static Rtos::Task trackerTask("Tracker", 2, 8000, [](Rtos::Task* task){
            while (true)
            {
                {
                    MeasurementList measurements = *measurementsValueMutex.get();
                    if (measurements.size() > 0)
                    {
                        Rtos::ValueMutex<TrackerMap>::Lock trackers = trackersValueMutex.get();
                        for (auto& tracker : *trackers)
                            tracker.second.track(measurements.front().value);
                    }
                }
                delay(1000);
            }
        });

        static Rtos::Task wifiTask("WiFi", 1, 5000, [](Rtos::Task* task){
            wl_status_t previousWifiStatus = WiFi.status();
            while (true)
            {
                wl_status_t wifiStatus = WiFi.status();
                if (wifiStatus != WL_CONNECTED)
                    WiFi.reconnect();

                if (wifiStatus == WL_CONNECTED && previousWifiStatus != WL_CONNECTED)
                {
                    json configJson = networkConfigResource.deserialize();
                    json& stationaryJson = configJson.at("stationary");
                    stationaryJson["ssid"] = WiFi.SSID().c_str();
                    stationaryJson["password"] = WiFi.psk().c_str();
                    json& ipConfigJson = stationaryJson.at("ipConfig");
                    ipConfigJson["ipAddress"] = WiFi.localIP().toString().c_str();
                    ipConfigJson["gatewayAddress"] = WiFi.gatewayIP().toString().c_str();
                    ipConfigJson["subnetMask"] = WiFi.subnetMask().toString().c_str();

                    networkConfigResource.serialize(configJson);

                    bool accesspointAlwaysActive = configJson.at("/accesspoint/alwaysActive"_json_pointer);
                    if (!accesspointAlwaysActive)
                        WiFi.mode(WIFI_STA);

                    Logger[LogLevel::Info]
                        << "(Re)connected to \""
                        << WiFi.SSID().c_str()
                        << "\", IP: "
                        << WiFi.localIP().toString().c_str()
                        << std::endl;
                }
                previousWifiStatus = wifiStatus;
                delay(10000);
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
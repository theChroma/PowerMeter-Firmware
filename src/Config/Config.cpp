#ifdef ESP32

#include "Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "Clock/DS3231/DS3231.h"
#include "Clock/SimulationClock/SimulationClock.h"
#include "MeasuringUnit/AcMeasuringUnit/AcMeasuringUnit.h"
#include "MeasuringUnit/SimulationMeasuringUnit/SimulationMeasuringUnit.h"
#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"
#include "Filesystem/Directory/LittleFsDirectory/LittleFsDirectory.h"
#include "Filesystem/File/LittleFsFile/LittleFsFile.h"
#include "Switch/NoSwitch/NoSwitch.h"
#include "Switch/Relay/Relay.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "Version/Version.h"
#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <tl/optional.hpp>
#include <fstream>
#include <functional>
#include <unordered_map>

namespace
{
    template<typename T>
    using ImplementationMap = std::unordered_map<std::string, std::function<T*(const json&)>>;


    template<typename T>
    T* configureImplementation(const json& configJson)
    {
        static tl::optional<T> implementation;
        implementation.emplace(configJson);
        return &implementation.value();
    }


    template<typename T>
    T* getSelectedImplementation(const json& configJson, const ImplementationMap<T>& implementations)
    {
        try
        {
            std::string key = configJson.at("selected");
            return implementations.at(key)(configJson.at("options").at(key));
        }
        catch (...)
        {
            ExceptionTrace::trace(SOURCE_LOCATION + "Failed to get selected Implementation");
            throw;
        }
    }


    IPAddress parseIpAddress(const std::string& ipString)
    {
        IPAddress ip;
        if (!ip.fromString(ipString.c_str()))
            throw std::runtime_error(SOURCE_LOCATION + "Could not parse \"" + ipString + "\" as IPv4");
        return ip;
    }


    LogStream configureLogStream(const json& configJson, std::ostream& stream)
    {
        LogLevel minLevel = configJson.at("minLevel").get<std::string>();
        LogLevel maxLevel = configJson.at("maxLevel").get<std::string>();
        bool showLevel = configJson.at("showLevel");
        return LogStream(minLevel, maxLevel, stream, showLevel);
    }


    json getConfigJson(JsonResource* configResource, const json& defaultConfigJson)
    {
        try
        {
            json configJson = configResource->deserializeOrGet([&configResource, &defaultConfigJson]{
                Logger[LogLevel::Info] << "Failed to deserialize config. Using default config." << std::endl;
                configResource->serialize(defaultConfigJson);
                return defaultConfigJson;
            });

            Version installedVersion(configJson.at("version"));
            Version latestVersion(defaultConfigJson.at("version"));
            if (installedVersion.major != latestVersion.major)
            {
                Logger[LogLevel::Info]
                    << "Version of current config (v"
                    << installedVersion
                    <<") is not compatible. Changing to v"
                    << latestVersion
                    << "."
                    << std::endl;
                configResource->serialize(defaultConfigJson);
                return defaultConfigJson;
            }
            return configJson;
        }
        catch (...)
        {
            ExceptionTrace::trace(SOURCE_LOCATION + "Failed to get config Json");
            throw;
        }
    }
}


json Config::getLoggerDefault() noexcept
{
    return {
        {"version", "0.0.0"},
        {"file", {
            {"filePath", "/Log/log.log"},
            {"showLevel", true},
            {"minLevel", "Error"},
            {"maxLevel", "Verbose"},
        }},
        {"console", {
            {"baudRate", 115200},
            {"showLevel", true},
            {"minLevel", "Error"},
            {"maxLevel", "Verbose"},
        }},
    };
}


void Config::configureLogger(JsonResource* configResource, AsyncWebServer* server)
{
    try
    {
        configureLogger(getConfigJson(configResource, getLoggerDefault()), server);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure logger");
        throw;
    }
}


void Config::configureLogger(const json& configJson, AsyncWebServer* server)
{
    Logger[LogLevel::Info] << "Configuring Logger..." << std::endl;
    try
    {
        Serial.begin(configJson.at("/console/baudRate"_json_pointer));

        static std::ofstream logFile;
        std::string logFilePath = configJson.at("/file/filePath"_json_pointer);
        logFile.open(logFilePath);
        server->on("/log", HTTP_GET, [logFilePath](AsyncWebServerRequest* request){
            logFile.close();
            request->send(LittleFS, logFilePath.c_str(), "text/plain");
            logFile.open(logFilePath, std::ios::app);
        });

        std::vector<LogStream> logStreams = {
            configureLogStream(configJson.at("console"), std::cout),
            configureLogStream(configJson.at("file"), logFile),
        };
        Logger = logStreams;
        Logger[LogLevel::Info] << "Logger configured sucessfully." << std::endl;
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure logger");
        throw;
    }
}


json Config::getMeasuringDefault() noexcept
{
    return {
        {"version", "0.0.0"},
        {"selected", "Ac"},
        {"options", {
            {"Simulation", {
                {"measuringRunTime_ms", 0},
                {"voltage", {
                    {"min", 220},
                    {"max", 240},
                }},
                {"current", {
                    {"min", 0.01},
                    {"max", 16},
                }},
                {"powerFactor", {
                    {"min", 0.5},
                    {"max", 1},
                }},
            }},
            {"Ac", {
                {"pins", {
                    {"voltage", 33},
                    {"current", 32},
                }},
                {"calibration", {
                    {"voltage", 536.9},
                    {"current", 15.7},
                    {"phase", -5.6},
                }},
            }},
        }},
    };
}


MeasuringUnit* Config::configureMeasuring(JsonResource* configResource)
{
    try
    {
        return configureMeasuring(getConfigJson(configResource, getMeasuringDefault()));
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure measuring unit");
        throw;
    }
}


MeasuringUnit* Config::configureMeasuring(const json& configJson)
{
    Logger[LogLevel::Info] << "Configuring measuring unit..." << std::endl;
    try
    {
        ImplementationMap<MeasuringUnit> measuringUnits = {
            {"Ac", configureImplementation<AcMeasuringUnit>},
            {"Simulation", configureImplementation<SimulationMeasuringUnit>},
        };
        return getSelectedImplementation<MeasuringUnit>(configJson, measuringUnits);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure measuring");
        throw;
    }
}


json Config::getClockDefault() noexcept
{
    return {
        {"version", "0.0.0"},
        {"selected", "DS3231"},
        {"options", {
            {"Simulation", {
                {"startTimestamp", 0},
                {"fastForward", 1},
            }},
            {"DS3231", nullptr},
        }},
    };
}


Clock* Config::configureClock(JsonResource* configResource)
{
    try
    {
        return configureClock(getConfigJson(configResource, getClockDefault()));
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure clock");
        throw;
    }
}


Clock* Config::configureClock(const json& configJson)
{
    Logger[LogLevel::Info] << "Configuring clock..." << std::endl;
    try
    {
        ImplementationMap<Clock> clocks = {
            {"DS3231", configureImplementation<DS3231>},
            {"Simulation", configureImplementation<SimulationClock>},
        };
        return getSelectedImplementation<Clock>(configJson, clocks);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure clock");
        throw;
    }
}


json Config::getSwitchDefault() noexcept
{
    return {
        {"version", "0.0.0"},
        {"selected", "Relay"},
        {"options", {
            {"Relay", {
                {"pin", 2},
                {"isNormallyOpen", true},
            }},
            {"None", nullptr},
        }},
    };
}


Switch* Config::configureSwitch(JsonResource* configResource)
{
    try
    {
        return configureSwitch(getConfigJson(configResource, getSwitchDefault()));
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure switch");
        throw;
    }
}


Switch* Config::configureSwitch(const json& configJson)
{
    Logger[LogLevel::Info] << "Configuring switch..." << std::endl;
    try
    {
        ImplementationMap<Switch> switches = {
            {"None", configureImplementation<NoSwitch>},
            {"Relay", configureImplementation<Relay>},
        };
        Switch* switchUnit = getSelectedImplementation<Switch>(configJson, switches);
        return switchUnit;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure switch");
        throw;
    }
}


json Config::getTrackersDefault() noexcept
{
    return {
        {"version", "0.0.0"},
        {"trackers", {
            {"3600_60", {
                {"title", "Last 60 Minutes"},
                {"duration_s", 3600},
                {"sampleCount", 60},
            }},
            {"86400_24", {
                {"title", "Last 24 Hours"},
                {"duration_s", 86400},
                {"sampleCount", 24},
            }},
            {"604800_7", {
                {"title", "Last 7 Days"},
                {"duration_s", 604800},
                {"sampleCount", 7},
            }},
            {"2592000_30", {
                {"title", "Last 30 Days"},
                {"duration_s", 2592000},
                {"sampleCount", 30},
            }},
            {"31104000_12", {
                {"title", "Last 12 Months"},
                {"duration_s", 31104000},
                {"sampleCount", 12},
            }},
        }},
    };
}


TrackerMap Config::configureTrackers(JsonResource* configResource, const Clock* clock)
{
    try
    {
        return configureTrackers(getConfigJson(configResource, getTrackersDefault()), clock);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure trackers");
        throw;
    }
}


TrackerMap Config::configureTrackers(const json& configJson, const Clock* clock)
{
    Logger[LogLevel::Info] << "Configuring trackers..." << std::endl;
    try
    {
        Filesystem::LittleFsDirectory trackersDirectory("/Trackers");
        Filesystem::Directory::Entries toBeRemovedEntries;
        try
        {
            toBeRemovedEntries = trackersDirectory.getEntries();
        }
        catch (...)
        {}

        TrackerMap trackers;
        for(const auto& trackerJson : configJson.at("trackers").items())
        {
            const std::string& trackerId = trackerJson.key();
            Filesystem::Directory::Entries::iterator toBeKeptEntryPosition = std::find_if(
                toBeRemovedEntries.begin(),
                toBeRemovedEntries.end(),
                [trackerId](const std::unique_ptr<Filesystem::Entry>& entry){
                    return entry->getName() == trackerId;
                }
            );
            if (toBeKeptEntryPosition != toBeRemovedEntries.end())
                toBeRemovedEntries.erase(toBeKeptEntryPosition);

            std::string trackerDirectoryPath = trackersDirectory.getPath() + '/' + trackerId;

            trackers.emplace(trackerId, Tracker(
                trackerJson.value().at("title"),
                trackerJson.value().at("duration_s"),
                trackerJson.value().at("sampleCount"),
                clock,
                std::unique_ptr<JsonResource>(
                    new BackedUpJsonResource(
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                                new Filesystem::LittleFsFile(trackerDirectoryPath + "/data.a.json")
                            )
                        ),
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                               new Filesystem::LittleFsFile(trackerDirectoryPath + "/data.b.json")
                            )
                        )
                    )
                ),
                std::unique_ptr<JsonResource>(
                    new BackedUpJsonResource(
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                                new Filesystem::LittleFsFile(trackerDirectoryPath + "/lastInputTimestamp.a.json")
                            )
                        ),
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                                new Filesystem::LittleFsFile(trackerDirectoryPath + "/lastInputTimestamp.b.json")
                            )
                        )
                    )
                ),
                std::unique_ptr<JsonResource>(
                    new BackedUpJsonResource(
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                                new Filesystem::LittleFsFile(trackerDirectoryPath + "/lastSampleTimestamp.a.json")
                            )
                        ),
                        BasicJsonResource(
                            std::unique_ptr<Filesystem::File>(
                                new Filesystem::LittleFsFile(trackerDirectoryPath + "/lastSampleTimestamp.b.json")
                            )
                        )
                    )
                ),
                AverageAccumulator(
                    std::unique_ptr<JsonResource>(
                        new BackedUpJsonResource(
                            BasicJsonResource(
                                std::unique_ptr<Filesystem::File>(
                                    new Filesystem::LittleFsFile(trackerDirectoryPath + "/accumulator.a.json")
                                )
                            ),
                            BasicJsonResource(
                                std::unique_ptr<Filesystem::File>(
                                    new Filesystem::LittleFsFile(trackerDirectoryPath + "/accumulator.b.json")
                                )
                            )
                        )
                    )
                )
            ));
        }
        Logger[LogLevel::Info] << "Trackers configured sucessfully." << std::endl;
        return trackers;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure trackers");
        throw;
    }
}


json Config::getNetworkDefault() noexcept
{
    std::stringstream hostname;
    hostname << "powermeter-" << std::hex << ESP.getEfuseMac();

    std::stringstream ssid;
    ssid << "Power Meter " << std::hex << ESP.getEfuseMac();

    return {
        {"version", "1.0.0"},
        {"hostname", hostname.str()},
        {"stationary", {
            {"macAddress", WiFi.macAddress().c_str()},
            {"ssid", ""},
            {"password", ""},
            {"ipMode", "DHCP"},
            {"ipConfig", {
                {"ipAddress", "0.0.0.0"},
                {"gatewayAddress", "0.0.0.0"},
                {"subnetMask", "0.0.0.0"},
            }},
        }},
        {"accesspoint", {
            {"macAddress", WiFi.softAPmacAddress().c_str()},
            {"alwaysActive", true},
            {"ssid", ssid.str()},
            {"password", "123456789"},
            {"ipConfig", {
                {"ipAddress", "192.168.4.1"}
            }},
        }},
    };
}


void Config::configureNetwork(JsonResource* configResource)
{
    try
    {
        json configJson = getConfigJson(configResource, getNetworkDefault());
        configureNetwork(&configJson);
        configResource->serialize(configJson);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure network");
        throw;
    }
}


void Config::configureNetwork(json* configJson)
{
    Logger[LogLevel::Info] << "Configuring network..." << std::endl;
    try
    {
        const std::string& hostname = configJson->at("hostname");;
        WiFi.setHostname(hostname.c_str());

        json& stationaryJson = configJson->at("stationary");
        json& accesspointJson = configJson->at("accesspoint");
        bool accesspointAlwaysActive = accesspointJson.at("alwaysActive");
        WiFi.mode(WIFI_AP_STA);
        {
            const std::string& ssid = stationaryJson.at("ssid");
            const std::string& password = stationaryJson.at("password");
            const std::string& ipMode = stationaryJson.at("ipMode");
            json& ipConfigJson = stationaryJson.at("ipConfig");
            const std::string& ipAddress = ipConfigJson.at("ipAddress");
            const std::string& gatewayAddress = ipConfigJson.at("gatewayAddress");
            const std::string& subnetMask = ipConfigJson.at("subnetMask");

            Logger[LogLevel::Info] << "Trying to connect to \"" << ssid << "\"..." << std::endl;

            if (stationaryJson.at("ipMode") == "Static")
                WiFi.config(parseIpAddress(ipAddress), parseIpAddress(gatewayAddress), parseIpAddress(subnetMask));
            WiFi.disconnect();
            WiFi.begin(ssid.c_str(), password.c_str());
            WiFi.setSleep(false);
            if(WiFi.waitForConnectResult(3000) == WL_CONNECTED)
            {
                ipConfigJson["ipAddress"] = WiFi.localIP().toString().c_str();
                ipConfigJson["gatewayAddress"] = WiFi.gatewayIP().toString().c_str();
                ipConfigJson["subnetMask"] = WiFi.subnetMask().toString().c_str();
                if (!accesspointAlwaysActive)
                    WiFi.mode(WIFI_STA);

                Logger[LogLevel::Info]
                    << "Connected to \""
                    << ssid
                    << "\", IP: "
                    << WiFi.localIP().toString().c_str()
                    << std::endl;
            }
            stationaryJson["macAddress"] = WiFi.macAddress().c_str();
        }
        {
            json& ssidJson = accesspointJson.at("ssid");
            const std::string& ssid = ssidJson;
            const std::string& password = accesspointJson.at("password");
            json& ipConfigJson = accesspointJson.at("ipConfig");
            IPAddress ipAddress = parseIpAddress(ipConfigJson.at("ipAddress"));

            if (WiFi.status() != WL_CONNECTED || accesspointAlwaysActive)
            {
                WiFi.softAP(ssid.c_str(), password.c_str());
                delay(100);
                WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
                ipConfigJson["ipAddress"] = WiFi.softAPIP().toString().c_str();
                Logger[LogLevel::Info]
                    << "Opened accespoint \""
                    << ssid
                    << "\", IP: "
                    << WiFi.softAPIP().toString().c_str()
                    << std::endl;
            }
            accesspointJson["macAddress"] = WiFi.softAPmacAddress().c_str();
        }

        MDNS.end();
        MDNS.begin(hostname.c_str());
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("powermeter", "tcp", 80);
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure network");
        throw;
    }
}

#endif
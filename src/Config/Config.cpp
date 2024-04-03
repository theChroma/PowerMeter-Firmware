#ifdef ESP32

#include "Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "Clock/DS3231/DS3231.h"
#include "Clock/SimulationClock/SimulationClock.h"
#include "MeasuringUnit/AcMeasuringUnit/AcMeasuringUnit.h"
#include "MeasuringUnit/SimulationMeasuringUnit/SimulationMeasuringUnit.h"
#include "JsonResource/BackedUpJsonResource/BackedUpJsonResource.h"
#include "Switch/NoSwitch/NoSwitch.h"
#include "Switch/Relay/Relay.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <tl/optional.hpp>
#include <fstream>
#include <functional>
#include <unordered_map>

using namespace PM;
using tl::optional;

namespace
{
    template<typename T>
    using ImplementaitonMap = std::unordered_map<std::string, std::function<T&(const json&)>>;


    template<typename T>
    std::reference_wrapper<T> configureImplementation(const json& configJson)
    {
        static optional<T> implementation;
        implementation.emplace(configJson);
        return implementation.value();
    }


    template<typename T>
    std::reference_wrapper<T> getSelectedImplementation(const JsonResource& configResource, const ImplementaitonMap<T>& implementations)
    {
        try
        {
            json configJson = configResource.deserialize();
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
}

void Config::configureLogger(const JsonResource& configResource, AsyncWebServer& server)
{
    try
    {
        Logger[LogLevel::Info] << "Configuring Logger..." << std::endl;

        try
        {
            json configJson = configResource.deserialize();
            Serial.begin(configJson.at("/console/baudRate"_json_pointer));

            static std::ofstream logFile;
            std::string logFilePath = configJson.at("/file/filePath"_json_pointer);
            logFile.open(logFilePath);
            server.on("/log", HTTP_GET, [logFilePath](AsyncWebServerRequest* request){
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
            ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Logger");
            throw;
        }
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Logger");
        throw;
    }
}


std::reference_wrapper<MeasuringUnit> Config::configureMeasuringUnit(const JsonResource& configResource)
{
    Logger[LogLevel::Info] << "Configuring measuring unit..." << std::endl;
    try
    {
        ImplementaitonMap<MeasuringUnit> measuringUnits = {
            {"Ac", configureImplementation<AcMeasuringUnit>},
            {"Simulation", configureImplementation<SimulationMeasuringUnit>},
        };
        return getSelectedImplementation<MeasuringUnit>(configResource, measuringUnits);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure measuring unit");
        throw;
    }
}


std::reference_wrapper<Clock> Config::configureClock(const JsonResource& configResource)
{
    Logger[LogLevel::Info] << "Configuring clock..." << std::endl;
    try
    {
        ImplementaitonMap<Clock> clocks = {
            {"DS3231", configureImplementation<DS3231>},
            {"Simulation", configureImplementation<SimulationClock>},
        };
        return getSelectedImplementation<Clock>(configResource, clocks);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure clock");
        throw;
    }
}


std::reference_wrapper<Switch> Config::configureSwitch(const JsonResource& configResource)
{
    Logger[LogLevel::Info] << "Configuring switch..." << std::endl;
    try
    {
        ImplementaitonMap<Switch> switches = {
            {"None", configureImplementation<NoSwitch>},
            {"Relay", configureImplementation<Relay>},
        };
        Switch& switchUnit = getSelectedImplementation<Switch>(configResource, switches);
        return switchUnit;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Switch");
        throw;
    }
}


TrackerMap Config::configureTrackers(const JsonResource& configResource, std::reference_wrapper<Clock> clock)
{
    Logger[LogLevel::Info] << "Configuring trackers..." << std::endl;
    TrackerMap trackers;
    try
    {
        json configJson = configResource.deserialize();
        for(const auto& trackerJson : configJson.items())
        {
            std::string key = trackerJson.key();
            std::stringstream trackerDirectory;
            trackerDirectory << "/Trackers/" << key << '/';

            trackers.emplace(key, Tracker(
                trackerJson.value().at("title"),
                trackerJson.value().at("duration_s"),
                trackerJson.value().at("sampleCount"),
                clock,
                std::shared_ptr<BackedUpJsonResource>(
                    new BackedUpJsonResource(trackerDirectory.str() + "data.json")
                ),
                std::shared_ptr<BackedUpJsonResource>(
                    new BackedUpJsonResource(trackerDirectory.str() + "lastInputTimestamp.json")
                ),
                std::shared_ptr<BackedUpJsonResource>(
                    new BackedUpJsonResource(trackerDirectory.str() + "lastSampleTimestamp.json")
                ),
                AverageAccumulator(
                    std::shared_ptr<BackedUpJsonResource>(
                        new BackedUpJsonResource(trackerDirectory.str() + "accumulator.json")
                    )
                )
            ));
        }
        Logger[LogLevel::Info] << "Trackers configured sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure trackers");
        throw;
    }
    return trackers;
}


void Config::configureNetwork(JsonResource &configResource)
{
    try
    {
        json configJson = configResource.deserialize();

        json& hostnameJson = configJson.at("hostname");
        if (hostnameJson.is_null())
        {
            std::stringstream hostname;
            hostname << "powermeter-" << std::hex << ESP.getEfuseMac();
            hostnameJson = hostname.str();
        }
        const std::string& hostname = hostnameJson;
        WiFi.setHostname(hostname.c_str());

        json& stationaryJson = configJson.at("stationary");
        json& accesspointJson = configJson.at("accesspoint");
        bool accesspointAlwaysActive = accesspointJson.at("alwaysActive");
        WiFi.mode(WIFI_AP_STA);
        {
            Logger[LogLevel::Info] << "Trying to connect to a stationary WiFi network..." << std::endl;
            const std::string& ssid = stationaryJson.at("ssid");
            const std::string& password = stationaryJson.at("password");
            const std::string& ipMode = stationaryJson.at("ipMode");
            json& ipConfigJson = stationaryJson.at("ipConfig");
            const std::string& ipAddress = ipConfigJson.at("ipAddress");
            const std::string& gatewayAddress = ipConfigJson.at("gatewayAddress");
            const std::string& subnetMask = ipConfigJson.at("subnetMask");
            stationaryJson["macAddress"] = WiFi.macAddress().c_str();

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
        }
        {
            json& ssidJson = accesspointJson.at("ssid");
            if (ssidJson.is_null())
            {
                std::stringstream ssid;
                ssid << "Power Meter " << std::hex << ESP.getEfuseMac();
                ssidJson = ssid.str();
            }
            const std::string& ssid = ssidJson;
            const std::string& password = accesspointJson.at("password");
            json& ipConfigJson = accesspointJson.at("ipConfig");
            IPAddress ipAddress = parseIpAddress(ipConfigJson.at("ipAddress"));
            accesspointJson["macAddress"] = WiFi.softAPmacAddress().c_str();

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
        }

        MDNS.end();
        MDNS.begin(hostname.c_str());
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("powermeter", "tcp", 80);

        configResource.serialize(configJson);
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Network");
        throw;
    }
}

#endif
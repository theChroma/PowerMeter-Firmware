#ifdef ESP32

#include "Config.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include "Clock/DS3231/DS3231.h"
#include "Clock/SimulationClock/SimulationClock.h"
#include "MeasuringUnit/AcMeasuringUnit/AcMeasuringUnit.h"
#include "MeasuringUnit/SimulationMeasuringUnit/SimulationMeasuringUnit.h"
#include "Switch/NoSwitch/NoSwitch.h"
#include "Switch/Relay/Relay.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <tl/optional.hpp>
#include <fstream>
#include <functional>
#include <unordered_map>

using namespace PM;
using tl::optional;

namespace
{
    std::ostream& getConsoleByName(const std::string& name)
    {
        std::unordered_map<std::string, std::ostream&> consoles = {
            {"stdout", std::cout},
            {"stderr", std::cerr},
            {"stdlog", std::clog},
        };
        return consoles.at(name);
    }

    LogStream configureLogStream(const json& configJson)
    {
        LogLevel minLevel = configJson.at("minLevel").get<std::string>();
        LogLevel maxLevel = configJson.at("maxLevel").get<std::string>();
        bool showLevel = configJson.at("showLevel");
        if (configJson.at("type") == "file")
        {
            static std::vector<std::ofstream> logFiles;
            logFiles.emplace_back(configJson.at("filePath"));
            return LogStream(minLevel, maxLevel, logFiles.back(), showLevel);
        }
        return LogStream(minLevel, maxLevel, getConsoleByName(configJson.at("console")), showLevel);
    }

    template<typename T>
    using ImplementaitonMap = std::unordered_map<std::string, std::function<T&(const json&)>>; 


    template<typename T>
    T& configureImplementation(const json& configJson)
    {
        static optional<T> implementation;
        implementation.emplace(configJson);
        return implementation.value();
    }


    template<typename T>
    T& getSelectedImplementation(const JsonResource& configResource, const ImplementaitonMap<T>& implementations)
    {
        json configJson = configResource.deserialize();
        std::string key = configJson.at("selected");
        return implementations.at(key)(configJson.at("options").at(key));
    }


    bool configureWiFiStationary(const json& configJson)
    {
        Logger[LogLevel::Info] << "Configuring WiFi in stationary mode..." << std::endl;
        try
        {
            const std::string& ssid = configJson.at("ssid");
            const std::string& password = configJson.at("password");
            const std::string& staticIP = configJson.at("staticIP");
            const std::string& gateway = configJson.at("gateway");
            const std::string& subnet = configJson.at("subnet");

            auto parseIP = [](const std::string& address)
            {
                IPAddress ip;
                if (!ip.fromString(address.c_str()))
                    Logger[LogLevel::Warning] << SOURCE_LOCATION << "Could not convert \"" << address << "\" to 'IPAdress'" << std::endl;
                return ip;
            };
            
            WiFi.disconnect(true);
            WiFi.config(parseIP(staticIP), parseIP(gateway), parseIP(subnet));
            WiFi.begin(ssid.c_str(), password.c_str());
            WiFi.setSleep(false);

            if(WiFi.waitForConnectResult(3000) == WL_CONNECTED)
                return true;
        }
        catch(...)
        {
            ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure WiFi in Stationary Mode");
            throw;
        }
        return false;
    }


    bool configureWifiAccesspoint(const json& configJson)
    {
        Logger[LogLevel::Info] << "Configuring WiFi as accesspoint..." << std::endl;
        try
        {
            const std::string& ssid = configJson.at("ssid");
            const std::string& password = configJson.at("password");
            return WiFi.softAP(ssid.c_str(), password.c_str());
        }
        catch(...)
        {
            ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure WiFi as accesspoint from");
            throw;
        }
    }
}

void Config::configureLogger(const JsonResource& configResource)
{
    try
    {
        Logger[LogLevel::Info] << "Configuring Logger..." << std::endl;

        json configJson = configResource.deserialize();
        Serial.begin(configJson.at("baudRate"));
        std::vector<LogStream> streams;
        for (const auto& streamJson : configJson.at("streams"))
        {
            streams.push_back(configureLogStream(streamJson));    
        }
        Logger = streams;

        Logger[LogLevel::Info] << "Logger configured sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Logger");
        throw;
    }
}


MeasuringUnit& Config::configureMeasuringUnit(const JsonResource& configResource)
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


Clock& Config::configureClock(const JsonResource& configResource)
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


Switch& Config::configureSwitch(const JsonResource& configResource)
{
    Logger[LogLevel::Info] << "Configuring switch..." << std::endl;
    try
    {
        ImplementaitonMap<Switch> switches = {
            {"None", configureImplementation<NoSwitch>}, 
            {"Relay", configureImplementation<Relay>}, 
        };
        return getSelectedImplementation<Switch>(configResource, switches);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Switch");
        throw;
    }
}


TrackerMap Config::configureTrackers(const JsonResource& configResource, Clock& clock)
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
                JsonResource(trackerDirectory.str() + "data.json"),
                JsonResource(trackerDirectory.str() + "timestamps.json#/lastInput"),
                JsonResource(trackerDirectory.str() + "timestamps.json#/lastSample"),
                AverageAccumulator(JsonResource(trackerDirectory.str() + "accumulator.json"))
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


void Config::configureWiFi(const JsonResource& configResource)
{
    Logger[LogLevel::Info] << "Configuring WiFi..." << std::endl;

    try
    {
        json configJson = configResource.deserialize();

        WiFi.mode(WiFiMode_t::WIFI_MODE_STA);
        if(configureWiFiStationary(configJson.at("sta")))
        {
            Logger[LogLevel::Info] 
                << "WiFi connected sucessfully to '"
                << configJson.at("sta").at("ssid")
                << "'. " 
                << "IP: http://" << WiFi.localIP().toString().c_str()
                << std::endl;
            
            MDNS.end();
            MDNS.begin("powermeter-abc");
            MDNS.addService("http", "tcp", 80);
            MDNS.addService("powermeter", "tcp", 80);
            return;
        }

        Logger[LogLevel::Info] 
            << "Couldn't connect to" 
            << configJson.at("sta").at("ssid")
            << ". Setting up Acesspoint..."
            << std::endl;

        WiFi.mode(WiFiMode_t::WIFI_MODE_AP);
        if(configureWifiAccesspoint(configJson.at("ap")))
        {
            Logger[LogLevel::Info] 
                << "WiFi configured sucessfully in Acesspoint Mode. Network Name: "
                << configJson.at("ap").at("ssid")
                << " IP: http://" << WiFi.softAPIP().toString().c_str()
                << std::endl;
            return;
        }

        throw std::runtime_error("Failed to configure WiFi");        
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure WiFi");
        throw;
    }
}

#endif
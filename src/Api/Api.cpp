#ifdef ESP32

#include "Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <functional>

using namespace PM;

namespace 
{
    RestAPI::JsonResponse handleGetJsonResource(const JsonResource& jsonResource)
    {
        return RestAPI::JsonResponse(jsonResource.deserialize());
    }


    RestAPI::JsonResponse handlePutJsonResource(const JsonResource& jsonResource, const json& requestJson)
    {
        jsonResource.serialize(requestJson);
        return RestAPI::JsonResponse(jsonResource.deserialize());
    }


    RestAPI::JsonResponse handlePatchJsonResource(const JsonResource& jsonResource, const json& requestJson, bool allowAdding = false)
    {
        json storedData = jsonResource.deserialize();
        size_t sizeBefore = storedData.size();
        storedData.merge_patch(requestJson);

        if(allowAdding && storedData.size() > sizeBefore)
            throw std::runtime_error("Adding properties using PATCH is not allowed here");

        jsonResource.serialize(storedData);
        return RestAPI::JsonResponse(jsonResource.deserialize());
    }
}

void Api::createSystemEndpoints(RestAPI& api)
{
    api.registerURI("/info", HTTP::Method::GET, [](json){
        json responseJson;
        responseJson["mac"] = ESP.getEfuseMac();
        std::stringstream firmwareVersion;
        firmwareVersion 
            << POWERMETER_FIRMWARE_VERSION_MAJOR << '.'
            << POWERMETER_FIRMWARE_VERSION_MINOR << '.'
            << POWERMETER_FIRMWARE_VERSION_PATCH;
        responseJson["firmware"] = firmwareVersion.str();
        responseJson["uptime_ms"] = millis();
        responseJson["filesystem"]["total_B"] = LittleFS.totalBytes();
        responseJson["filesystem"]["used_B"] = LittleFS.usedBytes();
        responseJson["heap"]["total_B"] = ESP.getHeapSize();
        responseJson["heap"]["used_B"] = ESP.getHeapSize() - ESP.getFreeHeap();
        return RestAPI::JsonResponse(responseJson); 
    });

    api.registerURI("/reboot", HTTP::Method::POST, [](json){
        Logger[LogLevel::Info] << "Rebooting PowerMeter..." << std::endl;
        ESP.restart();
        return RestAPI::JsonResponse(nullptr, HTTP::StatusCode::NoContent); 
    });
}


void Api::createLoggerEndpoints(RestAPI& api, const JsonResource& configResource)
{
    api.registerURI("/logger/config", HTTP::Method::GET, std::bind(handleGetJsonResource, configResource));
    api.registerURI("/logger/config", HTTP::Method::PATCH, [&configResource](const json& requestJson){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        Config::configureLogger(configResource);
        return response; 
    });
}


void Api::createMeasuringEndpoints(
    RestAPI& api,
    const JsonResource& configResource,
    MeasuringUnit& measuringUnit,
    Measurement& measurement
)
{
    api.registerURI("/measurements", HTTP::Method::GET, [&measuringUnit, &measurement](json){        
        return RestAPI::JsonResponse(measurement.toJson());
    });
    api.registerURI("/measuring/config", HTTP::Method::GET, std::bind(handleGetJsonResource, configResource));
    api.registerURI("/measuring/config", HTTP::Method::PATCH, [&configResource, &measuringUnit](const json& requestJson){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        measuringUnit = Config::configureMeasuringUnit(configResource);
        return response;
    });
}


void Api::createSwitchEndpoints(RestAPI& api, const JsonResource& configResource, Switch& switchUnit)
{
    api.registerURI("/switch", HTTP::Method::GET, [&switchUnit](json){
        json responseJson;
        tl::optional<bool> state = switchUnit.getState();
        if (state.has_value())
            responseJson = state.value();
        return RestAPI::JsonResponse(responseJson);
    });
    api.registerURI("/switch", HTTP::Method::PATCH, [&switchUnit](const json& requestJson){
        switchUnit.setState(requestJson);
        return requestJson;
    });
    api.registerURI("/switch/config", HTTP::Method::GET, std::bind(handleGetJsonResource, configResource));
    api.registerURI("/switch/config", HTTP::Method::PATCH, [configResource, &switchUnit](const json& requestJson){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        switchUnit = Config::configureSwitch(configResource);
        return response; 
    });
}


void Api::createTrackerEndpoints(RestAPI& api, const JsonResource& configResource, TrackerMap& trackers, Clock& clock)
{
    api.registerURI("/trackers", HTTP::Method::GET, [&trackers](json){
        json responseJson = json::object_t();
        for(const auto& tracker : trackers)
            responseJson[tracker.first] = tracker.second.getData();

        return RestAPI::JsonResponse(responseJson, HTTP::StatusCode::OK, {{"Content-Disposition", "inline"}});
    });
    api.registerURI("/trackers", HTTP::Method::PUT, [&trackers](const json& requestJson){
        for(const auto& requestItems : requestJson.items())
        {
            try
            {
                trackers.at(requestItems.key()).setData(requestItems.value());
            }
            catch(std::out_of_range)
            {
                ExceptionTrace::clear();
            }
        }
        return RestAPI::JsonResponse(requestJson);
    });
    api.registerURI("/trackers/config", HTTP::Method::GET, std::bind(handleGetJsonResource, configResource));
    api.registerURI("/trackers/config", HTTP::Method::POST, [&configResource, &trackers, &clock](const json& requestJson){
        json configJson = configResource.deserialize();
        std::stringstream key;
        key << requestJson.at("duration_s") << "_" << requestJson.at("sampleCount");
        configJson[key.str()] = requestJson;
        configResource.serialize(configJson);
        trackers = Config::configureTrackers(configResource, clock);
        return RestAPI::JsonResponse(configJson, HTTP::StatusCode::Created);
    });
    
    json trackersJson = configResource.deserialize();
    for(const auto& jsonTracker : trackersJson.items())
    {
        std::string key = jsonTracker.key();
        api.registerURI(
            std::string("/trackers/config/") + key,
            HTTP::Method::DELETE,
            [key, &configResource, &trackers, &clock](json){
                json configJson = configResource.deserialize();
                trackers.at(key).erase();
                trackers.erase(key);
                configJson.erase(key);
                configResource.serialize(configJson);
                trackers = Config::configureTrackers(configResource, clock);
                return RestAPI::JsonResponse(configJson);
            }
        );
    }
}


void Api::createWiFiEndpoints(RestAPI& api, const JsonResource& configResource)
{
    auto wifiModeToString = [](WiFiMode_t mode){
        switch(mode)
        {
            case WiFiMode_t::WIFI_MODE_AP:
                return "Acesspoint";
            case WiFiMode_t::WIFI_MODE_STA:
                return "Stationary";
            case WiFiMode_t::WIFI_MODE_APSTA:
                return "Acesspoint + Stationary";
            default:
                throw std::runtime_error("Invalid WiFi mode");
        }
    };

    api.registerURI("/wifi/config", HTTP::Method::GET, [wifiModeToString, &configResource](json){
        RestAPI::JsonResponse response = handleGetJsonResource(configResource);
        response.data.at("sta").erase("password");
        response.data["mode"] = wifiModeToString(WiFi.getMode());
        response.data["sta"]["ip"] = WiFi.localIP().toString().c_str();
        response.data["ap"]["ip"] = WiFi.softAPIP().toString().c_str();
        return response;
    });

    api.registerURI("/wifi/config", HTTP::Method::PATCH, [wifiModeToString, &configResource](const json& requestJson){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        Config::configureWiFi(configResource);
        response.data.at("sta").erase("password");
        response.data["mode"] = wifiModeToString(WiFi.getMode());
        response.data["sta"]["ip"] = WiFi.localIP().toString().c_str();
        response.data["ap"]["ip"] = WiFi.softAPIP().toString().c_str();
        return response;
    });
}

#endif
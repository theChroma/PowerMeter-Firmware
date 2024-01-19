#ifdef ESP32

#include "Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
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
        return jsonResource.deserialize();
    }


    RestAPI::JsonResponse handlePatchJsonResource(const JsonResource& jsonResource, const json& requestJson, bool allowAdding = false)
    {
        json storedJson = jsonResource.deserialize();
        size_t sizeBefore = storedJson.size();
        storedJson.merge_patch(requestJson);

        if(allowAdding && storedJson.size() > sizeBefore)
            throw std::runtime_error(SOURCE_LOCATION + "Adding properties using PATCH is not allowed here");

        jsonResource.serialize(storedJson);
        return jsonResource.deserialize();
    }
}

void Api::createSystemEndpoints(RestAPI& api)
{
    api.handle("/info", HTTP_GET, [](json, const Version& apiVersion){
        json responseJson;
        responseJson["mac"] = ESP.getEfuseMac();
        std::stringstream firmwareVersion;

        responseJson["firmware"] = Version(POWERMETER_FIRMWARE_VERSION_MAJOR, POWERMETER_FIRMWARE_VERSION_MINOR, POWERMETER_FIRMWARE_VERSION_PATCH);
        responseJson["uptime_ms"] = millis();
        responseJson["filesystem"]["total_B"] = LittleFS.totalBytes();
        responseJson["filesystem"]["used_B"] = LittleFS.usedBytes();
        responseJson["heap"]["total_B"] = ESP.getHeapSize();
        responseJson["heap"]["used_B"] = ESP.getHeapSize() - ESP.getFreeHeap();
        return RestAPI::JsonResponse(responseJson);
    });

    api.handle("/reboot", HTTP_POST, [](json, Version){
        xTaskCreate([](void*){
            delay(1000);
            Logger[LogLevel::Info] << "Rebooting PowerMeter..." << std::endl;
            ESP.restart();
            vTaskDelete(nullptr);
        }, "reboot", 1500, nullptr, 1, nullptr);
        return RestAPI::JsonResponse(nullptr, 204);
    });
}


void Api::createLoggerEndpoints(RestAPI& api, const JsonResource& configResource, AsyncWebServer& server)
{
    api.handle("/logger/config", HTTP_GET, [configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    api.handle("/logger/config", HTTP_PATCH, [configResource, &server](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        Config::configureLogger(configResource, server);
        return response;
    });
}


void Api::createMeasuringEndpoints(
    RestAPI& api,
    const JsonResource& configResource,
    std::reference_wrapper<MeasuringUnit>& measuringUnit,
    std::reference_wrapper<Measurement>& measurement
)
{
    api.handle("/measure", HTTP_GET, [&measurement](json, Version){
        return measurement.get().toJson();
    });
    api.handle("/measuring/config", HTTP_GET, [configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    api.handle("/measuring/config", HTTP_PATCH, [configResource, &measuringUnit](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        measuringUnit = Config::configureMeasuringUnit(configResource);
        return response;
    });
}

void Api::createSwitchEndpoints(RestAPI& api, const JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit)
{
    api.handle("/switch", HTTP_GET, [&switchUnit](json, Version){
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });
    api.handle("/switch", HTTP_PATCH, [&switchUnit](const json& requestJson, Version){
        switchUnit.get().setState(requestJson);
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });
    api.handle("/switch/config", HTTP_GET, [configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    api.handle("/switch/config", HTTP_PATCH, [configResource, &switchUnit](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        switchUnit = Config::configureSwitch(configResource);
        return response;
    });
}

void Api::createClockEndpoints(RestAPI &api, const JsonResource& configResource, std::reference_wrapper<Clock>& clock)
{
    api.handle("/clock/config", HTTP_GET, [configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    api.handle("/clock/config", HTTP_PATCH, [configResource, &clock](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        clock = Config::configureClock(configResource);
        return response;
    });
}

void Api::createTrackerEndpoints(RestAPI& api, const JsonResource& configResource, TrackerMap& trackers, Clock& clock)
{
    api.handle("/trackers", HTTP_GET, [&trackers](json, Version){
        json responseJson = json::object_t();
        for(const auto& tracker : trackers)
            responseJson[tracker.first] = tracker.second.getData();

        return RestAPI::JsonResponse(responseJson);
    });
    api.handle("/trackers", HTTP_PUT, [&trackers](const json& requestJson, Version){
        json responseJson = json::object_t();
        for(const auto& requestJsonItems : requestJson.items())
        {
            const std::string& trackerId = requestJsonItems.key();
            if (trackers.find(trackerId) != trackers.end())
            {
                Tracker tracker = trackers.at(trackerId);
                tracker.setData(requestJsonItems.value());
                responseJson[trackerId] = tracker.getData();
            }
        }
        return responseJson;
    });
    api.handle("/trackers/config", HTTP_GET, [configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    api.handle("/trackers/config", HTTP_POST, [configResource, &trackers, &clock](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        std::stringstream key;
        key << requestJson.at("duration_s") << "_" << requestJson.at("sampleCount");
        configJson[key.str()] = requestJson;
        configResource.serialize(configJson);
        trackers = Config::configureTrackers(configResource, clock);
        return RestAPI::JsonResponse(configJson, 201);
    });

    json trackersJson = configResource.deserialize();
    for(const auto& jsonTracker : trackersJson.items())
    {
        std::string key = jsonTracker.key();
        api.handle(
            std::string("/trackers/config/") + key,
            HTTP_DELETE,
            [key, configResource, &trackers, &clock](json, Version){
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

    api.handle("/wifi/config", HTTP_GET, [wifiModeToString, configResource](json, Version){
        RestAPI::JsonResponse response = handleGetJsonResource(configResource);
        response.data.at("sta").erase("password");
        response.data["mode"] = wifiModeToString(WiFi.getMode());
        response.data["sta"]["ip"] = WiFi.localIP().toString().c_str();
        response.data["ap"]["ip"] = WiFi.softAPIP().toString().c_str();
        return response;
    });

    api.handle("/wifi/config", HTTP_PATCH, [wifiModeToString, configResource](const json& requestJson, Version){
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
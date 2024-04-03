#ifdef ESP32

#include "Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Rtos/Rtos.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <functional>

using namespace PM;

namespace
{
    RestAPI::JsonResponse handleGetJsonResource(JsonResource& jsonResource)
    {
        return RestAPI::JsonResponse(jsonResource.deserialize());
    }


    RestAPI::JsonResponse handlePutJsonResource(JsonResource& jsonResource, const json& requestJson)
    {
        jsonResource.serialize(requestJson);
        return jsonResource.deserialize();
    }

    size_t getJsonSizeRecursive(const json& data)
    {
        if (data.is_object())
        {
            size_t size = 0;
            for (const auto& child : data.items())
                size += getJsonSizeRecursive(child.value());
            return size;
        }
        else if (data.is_array())
        {
            size_t size = data.size();
            for (const auto& element : data)
                size += getJsonSizeRecursive(element);
            return size;
        }
        else
        {
            return data.size();
        }
    }

    RestAPI::JsonResponse handlePatchJsonResource(JsonResource& jsonResource, const json& requestJson, bool allowAdding = false)
    {
        json storedJson = jsonResource.deserialize();
        size_t sizeBefore = getJsonSizeRecursive(storedJson);
        storedJson.merge_patch(requestJson);

        if(!allowAdding && getJsonSizeRecursive(storedJson) > sizeBefore)
            throw std::runtime_error(SOURCE_LOCATION + "Adding properties using PATCH is not allowed here");

        jsonResource.serialize(storedJson);
        return jsonResource.deserialize();
    }
}


Api::Api(RestAPI &api) : m_restApi(api)
{}


void Api::createSystemEndpoints(const Version& firmwareVersion, const Version& apiVersion)
{
    m_restApi.handle("/info", HTTP_GET, [firmwareVersion, apiVersion](json, Version){
        json versionsJson;
        versionsJson["firmware"] = firmwareVersion;
        versionsJson["api"] = apiVersion;

        json statisticsJson;
        statisticsJson["filesystem"]["total_B"] = LittleFS.totalBytes();
        statisticsJson["filesystem"]["used_B"] = LittleFS.usedBytes();
        statisticsJson["heap"]["used_B"] = ESP.getHeapSize() - ESP.getMinFreeHeap();
        statisticsJson["heap"]["total_B"] = ESP.getHeapSize();

        json responseJson;
        std::stringstream chipdId;
        chipdId << std::hex << ESP.getEfuseMac();
        responseJson["chipId"] = chipdId.str();
        responseJson["uptime_s"] = millis() / 1000.0;
        // responseJson["network"] = networkJson;
        responseJson["versions"] = versionsJson;
        responseJson["statistics"] = statisticsJson;
        return responseJson;
    });
    m_restApi.handle("/reboot", HTTP_POST, [](json, Version){
        return RestAPI::JsonResponse(nullptr, 204, {}, []{
            Logger[LogLevel::Info] << "Rebooting PowerMeter..." << std::endl;
            ESP.restart();
        });
    });
}


void Api::createLoggerEndpoints(JsonResource& configResource, AsyncWebServer& server)
{
    m_restApi.handle("/logger/config", HTTP_GET, [&configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    m_restApi.handle("/logger/config", HTTP_PATCH, [&configResource, &server](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        Config::configureLogger(configResource, server);
        return response;
    });
}


void Api::createMeasuringEndpoints(
    JsonResource& configResource,
    std::reference_wrapper<MeasuringUnit>& measuringUnit,
    std::reference_wrapper<Measurement>& measurement
)
{
    m_restApi.handle("/measurement", HTTP_GET, [&measurement](json, Version){
        return measurement.get().toJson();
    });
    m_restApi.handle("/measuring/config", HTTP_GET, [&configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    m_restApi.handle("/measuring/config", HTTP_PATCH, [&configResource, &measuringUnit](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        measuringUnit = Config::configureMeasuringUnit(configResource);
        return response;
    });
}


void Api::createSwitchEndpoints(JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit)
{
    m_restApi.handle("/switch", HTTP_GET, [&switchUnit](json, Version){
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });
    m_restApi.handle("/switch", HTTP_PATCH, [&switchUnit](const json& requestJson, Version){
        switchUnit.get().setState(requestJson);
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });
    m_restApi.handle("/switch/config", HTTP_GET, [&configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    m_restApi.handle("/switch/config", HTTP_PATCH, [&configResource, &switchUnit](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        switchUnit = Config::configureSwitch(configResource);
        return response;
    });
}


void Api::createClockEndpoints(JsonResource& configResource, std::reference_wrapper<Clock>& clock)
{
    m_restApi.handle("/clock/config", HTTP_GET, [&configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    m_restApi.handle("/clock/config", HTTP_PATCH, [&configResource, &clock](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        clock = Config::configureClock(configResource);
        return response;
    });
}


void Api::createTrackerEndpoints(JsonResource& configResource, TrackerMap& trackers, Clock& clock)
{
    m_restApi.handle("/trackers", HTTP_GET, [&trackers](json, Version){
        json responseJson = json::object_t();
        for(const auto& tracker : trackers)
            responseJson[tracker.first] = tracker.second.getData();
        return RestAPI::JsonResponse(responseJson);
    });
    m_restApi.handle("/trackers", HTTP_PUT, [&trackers](const json& requestJson, Version){
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
    m_restApi.handle("/trackers/config", HTTP_GET, [&configResource](json, Version){
        return handleGetJsonResource(configResource);
    });
    m_restApi.handle("/trackers/config", HTTP_POST, [&configResource, &trackers, &clock](const json& requestJson, Version){
        std::lock_guard<std::mutex> lock(Rtos::trackerAccess);
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
        m_restApi.handle(
            std::string("/trackers/config/") + key,
            HTTP_DELETE,
            [key, &configResource, &trackers, &clock](json, Version){
                std::lock_guard<std::mutex> lock(Rtos::trackerAccess);
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


void Api::createNetworkEndpoints(JsonResource &configResource)
{
    m_restApi.handle("/network/config", HTTP_GET, [&configResource](json, Version){
        RestAPI::JsonResponse response = handleGetJsonResource(configResource);
        response.data["stationary"]["macAddress"] = WiFi.macAddress().c_str();
        response.data["acesspoint"]["macAddress"] = WiFi.softAPmacAddress().c_str();
        return response;
    });
    m_restApi.handle("/network/config", HTTP_PATCH, [&configResource](const json& requestJson, Version){
        RestAPI::JsonResponse response = handlePatchJsonResource(configResource, requestJson);
        response.doAfterSend = [&configResource]{
            Config::configureNetwork(configResource);
        };
        return response;
    });
}

#endif
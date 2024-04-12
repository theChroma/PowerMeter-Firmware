#ifdef ESP32

#include "Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <functional>
#include <vector>


namespace
{
    RestApi::JsonResponse getJsonResource(JsonResource& jsonResource)
    {
        return RestApi::JsonResponse(jsonResource.deserialize());
    }


    RestApi::JsonResponse putJsonResource(JsonResource& jsonResource, const json& requestJson)
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


    void patchJson(
        json& targetJson,
        const json& patchJson,
        const std::vector<json::json_pointer>& readonlyItems = {"/version"_json_pointer},
        bool allowAdding = false
    )
    {
        for (const auto& readonlyItem : readonlyItems)
        {
            if (patchJson.contains(readonlyItem))
                throw std::runtime_error(SOURCE_LOCATION + "Property at \"" + readonlyItem.to_string() + "\" is readonly");
        }

        size_t sizeBefore = getJsonSizeRecursive(targetJson);

        targetJson.merge_patch(patchJson);

        if(!allowAdding && getJsonSizeRecursive(targetJson) > sizeBefore)
            throw std::runtime_error(SOURCE_LOCATION + "Adding properties using PATCH is not allowed");
    }
}


void Api::createSystemEndpoints(RestApi& restApi, const Version& firmwareVersion, const Version& apiVersion)
{
    restApi.handle("/info", HTTP_GET, [firmwareVersion, apiVersion](json, Version){
        std::stringstream chipdId;
        chipdId << std::hex << ESP.getEfuseMac();
        return json {
            {"chipId", chipdId.str()},
            {"uptime_s", millis() / 1000.0},
            {"versions", {
                {"firmware", std::string(firmwareVersion)},
                {"api", std::string(firmwareVersion)},
            }},
            {"statistics", {
                {"filesystem", {
                    {"totalBytes", LittleFS.totalBytes()},
                    {"usedBytes", LittleFS.usedBytes()},
                }},
                {"heap", {
                    {"totalBytes", ESP.getHeapSize() - ESP.getMinFreeHeap()},
                    {"usedBytes", ESP.getHeapSize()},
                }},
            }}
        };
    });

    restApi.handle("/reboot", HTTP_POST, [](json, Version){
        return RestApi::JsonResponse(nullptr, 204, {}, []{
            Logger[LogLevel::Info] << "Rebooting PowerMeter..." << std::endl;
            ESP.restart();
        });
    });
}


void Api::createLoggerEndpoints(RestApi& restApi, JsonResource& configResource, AsyncWebServer& server)
{
    restApi.handle("/logger/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/logger/config", HTTP_PATCH, [&configResource, &server](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        patchJson(configJson, requestJson);
        Config::configureLogger(configJson, server);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/logger/config/default", HTTP_GET, [](json, Version){
        return Config::getLoggerDefault();
    });

    restApi.handle("/logger/config/restore-default", HTTP_POST, [&configResource, &server](json, Version){
        json defaultConfigJson = Config::getLoggerDefault();
        Config::configureLogger(defaultConfigJson, server);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createMeasuringEndpoints(
    RestApi& restApi,
    JsonResource& configResource,
    std::reference_wrapper<MeasuringUnit>& measuringUnit,
    const Rtos::ValueMutex<MeasurementList>& sharedMeasurements
)
{
    restApi.handle("/measurements", HTTP_GET, [&sharedMeasurements](json, Version){
        json responseJson = json::array_t();
        MeasurementList measurements = sharedMeasurements.get();
        for (const auto& measurement : measurements)
            responseJson.push_back(measurement.toJson());
        return responseJson;
    });

    restApi.handle("/measuring/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/measuring/config", HTTP_PATCH, [&configResource, &measuringUnit](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        patchJson(configJson, requestJson);
        measuringUnit = Config::configureMeasuring(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/measuring/config/default", HTTP_GET, [](json, Version){
        return Config::getMeasuringDefault();
    });

    restApi.handle("/measuring/config/restore-default", HTTP_POST, [&configResource, &measuringUnit](json, Version){
        json defaultConfigJson = Config::getMeasuringDefault();
        measuringUnit = Config::configureMeasuring(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createSwitchEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit)
{
    restApi.handle("/switch", HTTP_GET, [&switchUnit](json, Version){
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });

    restApi.handle("/switch", HTTP_PATCH, [&switchUnit](const json& requestJson, Version){
        switchUnit.get().setState(requestJson);
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });

    restApi.handle("/switch/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/switch/config", HTTP_PATCH, [&configResource, &switchUnit](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        patchJson(configJson, requestJson);
        switchUnit = Config::configureSwitch(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/switch/config/default", HTTP_GET, [](json, Version){
        return Config::getSwitchDefault();
    });

    restApi.handle("/switch/config/restore-default", HTTP_POST, [&configResource, &switchUnit](json, Version){
        json defaultConfigJson = Config::getSwitchDefault();
        switchUnit = Config::configureSwitch(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createClockEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Clock>& clock)
{
    restApi.handle("/clock/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/clock/config", HTTP_PATCH, [&configResource, &clock](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        patchJson(configJson, requestJson);
        clock = Config::configureClock(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/clock/config/default", HTTP_GET, [](json, Version){
        return Config::getClockDefault();
    });

    restApi.handle("/clock/config/restore-default", HTTP_POST, [&configResource, &clock](json, Version){
        json defaultConfigJson = Config::getClockDefault();
        clock = Config::configureClock(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createTrackerEndpoints(
    RestApi& restApi,
    JsonResource& configResource,
    Rtos::ValueMutex<TrackerMap>& sharedTrackers,
    Clock& clock
)
{
    restApi.handle("/trackers", HTTP_GET, [&sharedTrackers](json, Version){
        TrackerMap trackers = sharedTrackers;
        json responseJson = json::object_t();
        for(const auto& tracker : trackers)
            responseJson[tracker.first] = tracker.second.getData();
        return RestApi::JsonResponse(responseJson);
    });

    restApi.handle("/trackers", HTTP_PUT, [&sharedTrackers](const json& requestJson, Version){
        json responseJson = json::object_t();
        for(const auto& requestJsonItems : requestJson.items())
        {
            TrackerMap trackers = sharedTrackers;
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

    restApi.handle("/trackers/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/trackers/config", HTTP_PATCH, [&configResource, &clock, &sharedTrackers](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        patchJson(configJson, requestJson);
        sharedTrackers = Config::configureTrackers(configJson, clock);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/trackers/config", HTTP_POST, [&configResource, &sharedTrackers, &clock](const json& requestJson, Version){
        json configJson = configResource.deserialize();
        std::stringstream key;
        key << requestJson.at("duration_s") << "_" << requestJson.at("sampleCount");
        configJson["trackers"][key.str()] = requestJson;
        sharedTrackers = Config::configureTrackers(configJson, clock);
        configResource.serialize(configJson);
        return RestApi::JsonResponse(configJson, 201);
    });

    json trackersJson = configResource.deserialize();
    for(const auto& jsonTracker : trackersJson.items())
    {
        std::string key = jsonTracker.key();
        restApi.handle(
            std::string("/trackers/config/") + key,
            HTTP_DELETE,
            [key, &configResource, &sharedTrackers, &clock](json, Version){
                json configJson = configResource.deserialize();
                sharedTrackers.get().at(key).erase();
                configJson.at("trackers").erase(key);
                sharedTrackers = Config::configureTrackers(configJson, clock);
                configResource.serialize(configJson);
                return RestApi::JsonResponse(configJson);
            }
        );
    }

    restApi.handle("/trackers/config/default", HTTP_GET, [](json, Version){
        return Config::getTrackersDefault();
    });

    restApi.handle("/trackers/config/restore-default", HTTP_POST, [&configResource, &sharedTrackers, &clock](json, Version){
        json defaultConfigJson = Config::getTrackersDefault();
        sharedTrackers = Config::configureTrackers(defaultConfigJson, clock);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createNetworkEndpoints(RestApi& restApi, JsonResource &configResource)
{
    restApi.handle("/network/config", HTTP_GET, [&configResource](json, Version){
        return getJsonResource(configResource);
    });

    restApi.handle("/network/config", HTTP_PATCH, [&configResource](const json& requestJson, Version){
        RestApi::JsonResponse response = configResource.deserialize();
        patchJson(response.data, requestJson, {
            "/version"_json_pointer,
            "/stationary/macAddress"_json_pointer,
            "/accesspoint/macAddress"_json_pointer,
        });
        response.doAfterSend = [&configResource, response]{
            json configJson = response.data;
            Config::configureNetwork(configJson);
            configResource.serialize(configJson);
        };
        return response;
    });

    restApi.handle("/network/config/default", HTTP_GET, [](json, Version){
        return Config::getNetworkDefault();
    });

    restApi.handle("/network/config/restore-default", HTTP_POST, [&configResource](json, Version){
        json defaultConfigJson = Config::getNetworkDefault();
        Config::configureNetwork(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}

#endif
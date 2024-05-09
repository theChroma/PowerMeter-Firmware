#ifdef ESP32

#include "Api.h"
#include "Config/Config.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "WifiScan/WifiScan.h"
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
    restApi.handle("/info", HTTP_GET, [firmwareVersion, apiVersion](RestApi::JsonRequest){
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
                    {"totalBytes", ESP.getHeapSize()},
                    {"usedBytes", ESP.getHeapSize() - ESP.getMinFreeHeap()},
                }},
            }}
        };
    });

    restApi.handle("/reboot", HTTP_POST, [](RestApi::JsonRequest){
        return RestApi::JsonResponse(nullptr, 204, {}, []{
            Logger[LogLevel::Info] << "Rebooting PowerMeter..." << std::endl;
            ESP.restart();
        });
    });
}


void Api::createLoggerEndpoints(RestApi& restApi, JsonResource& configResource, AsyncWebServer& server)
{
    restApi.handle("/logger/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/logger/config", HTTP_PATCH, [&configResource, &server](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        patchJson(configJson, request.data);
        Config::configureLogger(configJson, server);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/logger/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getLoggerDefault();
    });

    restApi.handle("/logger/config/restore-default", HTTP_POST, [&configResource, &server](RestApi::JsonRequest){
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
    const Rtos::ValueMutex<MeasurementList>& measurementsValueMutex
)
{
    restApi.handle("/measurements", HTTP_GET, [&measurementsValueMutex](RestApi::JsonRequest){
        json responseJson = json::array_t();
        MeasurementList measurements = measurementsValueMutex.get();
        for (const auto& measurement : measurements)
            responseJson.push_back(measurement.toJson());
        return responseJson;
    });

    restApi.handle("/measuring/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/measuring/config", HTTP_PATCH, [&configResource, &measuringUnit](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        patchJson(configJson, request.data);
        measuringUnit = Config::configureMeasuring(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/measuring/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getMeasuringDefault();
    });

    restApi.handle("/measuring/config/restore-default", HTTP_POST, [&configResource, &measuringUnit](RestApi::JsonRequest){
        json defaultConfigJson = Config::getMeasuringDefault();
        measuringUnit = Config::configureMeasuring(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createSwitchEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit)
{
    restApi.handle("/switch", HTTP_GET, [&switchUnit](RestApi::JsonRequest){
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });

    restApi.handle("/switch", HTTP_PATCH, [&switchUnit](const RestApi::JsonRequest& request){
        switchUnit.get().setState(request.data);
        json responseJson;
        tl::optional<bool> state = switchUnit.get().getState();
        if (state.has_value())
            responseJson = state.value();
        return responseJson;
    });

    restApi.handle("/switch/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/switch/config", HTTP_PATCH, [&configResource, &switchUnit](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        patchJson(configJson, request.data);
        switchUnit = Config::configureSwitch(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/switch/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getSwitchDefault();
    });

    restApi.handle("/switch/config/restore-default", HTTP_POST, [&configResource, &switchUnit](RestApi::JsonRequest){
        json defaultConfigJson = Config::getSwitchDefault();
        switchUnit = Config::configureSwitch(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createClockEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Clock>& clock)
{
    restApi.handle("/clock/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/clock/config", HTTP_PATCH, [&configResource, &clock](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        patchJson(configJson, request.data);
        clock = Config::configureClock(configJson);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/clock/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getClockDefault();
    });

    restApi.handle("/clock/config/restore-default", HTTP_POST, [&configResource, &clock](RestApi::JsonRequest){
        json defaultConfigJson = Config::getClockDefault();
        clock = Config::configureClock(defaultConfigJson);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createTrackerEndpoints(
    RestApi& restApi,
    JsonResource& configResource,
    Rtos::ValueMutex<TrackerMap>& trackersValueMutex,
    Clock& clock
)
{
    restApi.handle("/trackers", HTTP_GET, [&trackersValueMutex](RestApi::JsonRequest){
        TrackerMap trackers = trackersValueMutex;
        json responseJson = json::object_t();
        for(const auto& tracker : trackers)
            responseJson[tracker.first] = tracker.second.getData();
        return RestApi::JsonResponse(responseJson);
    });

    restApi.handle("/trackers", HTTP_PUT, [&trackersValueMutex](const RestApi::JsonRequest& request){
        json responseJson = json::object_t();
        for(const auto& requestJsonItems : request.data.items())
        {
            TrackerMap trackers = trackersValueMutex;
            const std::string& trackerId = requestJsonItems.key();
            if (trackers.find(trackerId) != trackers.end())
            {
                Tracker& tracker = trackers.at(trackerId);
                tracker.setData(requestJsonItems.value());
                responseJson[trackerId] = tracker.getData();
            }
        }
        return responseJson;
    });

    restApi.handle("/trackers/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/trackers/config", HTTP_PATCH, [&configResource, &clock, &trackersValueMutex](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        patchJson(configJson, request.data);
        trackersValueMutex = Config::configureTrackers(configJson, clock);
        configResource.serialize(configJson);
        return configJson;
    });

    restApi.handle("/trackers/config", HTTP_POST, [&configResource, &trackersValueMutex, &clock](const RestApi::JsonRequest& request){
        json configJson = configResource.deserialize();
        std::stringstream key;
        key << request.data.at("duration_s") << "_" << request.data.at("sampleCount");
        configJson["trackers"][key.str()] = request.data;
        trackersValueMutex = Config::configureTrackers(configJson, clock);
        configResource.serialize(configJson);
        return RestApi::JsonResponse(configJson, 201);
    });

    restApi.handle("/trackers/config/(.*)", HTTP_DELETE,
        [&configResource, &trackersValueMutex, &clock](const RestApi::JsonRequest& request){
            json configJson = configResource.deserialize();
            std::string trackerId = request.serverRequest.pathArg(1).c_str();
            if (!configJson.at("trackers").erase(trackerId))
                throw std::runtime_error(SOURCE_LOCATION + " \"" + trackerId + "\" is not a valid tracker ID");
            trackersValueMutex = Config::configureTrackers(configJson, clock);
            configResource.serialize(configJson);
            return RestApi::JsonResponse(configJson);
        }
    );

    restApi.handle("/trackers/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getTrackersDefault();
    });

    restApi.handle("/trackers/config/restore-default", HTTP_POST, [&configResource, &trackersValueMutex, &clock](RestApi::JsonRequest){
        json defaultConfigJson = Config::getTrackersDefault();
        trackersValueMutex = Config::configureTrackers(defaultConfigJson, clock);
        configResource.serialize(defaultConfigJson);
        return defaultConfigJson;
    });
}


void Api::createNetworkEndpoints(RestApi& restApi, JsonResource &configResource)
{
    restApi.handle("/network/config", HTTP_GET, [&configResource](RestApi::JsonRequest){
        return getJsonResource(configResource);
    });

    restApi.handle("/network/config", HTTP_PATCH, [&configResource](const RestApi::JsonRequest& request){
        RestApi::JsonResponse response = configResource.deserialize();
        patchJson(response.data, request.data, {
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

    restApi.handle("/network/config/default", HTTP_GET, [](RestApi::JsonRequest){
        return Config::getNetworkDefault();
    });

    restApi.handle("/network/config/restore-default", HTTP_POST, [&configResource](RestApi::JsonRequest){
        RestApi::JsonResponse response = Config::getNetworkDefault();
        Logger[LogLevel::Debug] << response.data.dump(2, ' ') << std::endl;
        response.doAfterSend = [&configResource, response]{
            json configJson = response.data;
            Config::configureNetwork(configJson);
            configResource.serialize(configJson);
        };
        return response;
    });

    restApi.handle("/network/scan", HTTP_GET, [](RestApi::JsonRequest){
        return WifiScan().toJson();
    });
}

#endif
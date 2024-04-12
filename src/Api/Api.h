#pragma once

#include "RestAPI/RestAPI.h"
#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Switch/Switch.h"
#include "Tracker/Tracker.h"
#include "Rtos/ValueMutex/ValueMutex.h"


namespace Api
{
    void createSystemEndpoints(RestApi& restApi, const Version& firmwareVersion, const Version& apiVersion);
    void createLoggerEndpoints(RestApi& restApi, JsonResource& configResource, AsyncWebServer &server);
    void createSwitchEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit);
    void createClockEndpoints(RestApi& restApi, JsonResource& configResource, std::reference_wrapper<Clock>& clock);
    void createNetworkEndpoints(RestApi& restApi, JsonResource& configResource);
    void createTrackerEndpoints(
        RestApi& restApi,
        JsonResource& configResource,
        Rtos::ValueMutex<TrackerMap>& sharedTrackers,
        Clock& clock
    );
    void createMeasuringEndpoints(
        RestApi& restApi,
        JsonResource& configResource,
        std::reference_wrapper<MeasuringUnit>& measuringUnit,
        const Rtos::ValueMutex<MeasurementList>& sharedMeasurements
    );
};
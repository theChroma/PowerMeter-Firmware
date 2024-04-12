#pragma once

#include "RestAPI/RestAPI.h"
#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Switch/Switch.h"
#include "Tracker/Tracker.h"
#include "Rtos/ValueMutex/ValueMutex.h"


class Api
{
public:
    Api(RestApi& api);
    void createSystemEndpoints(const Version& firmwareVersion, const Version& apiVersion);
    void createLoggerEndpoints(JsonResource& configResource, AsyncWebServer &server);
    void createSwitchEndpoints(JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit);
    void createClockEndpoints(JsonResource& configResource, std::reference_wrapper<Clock>& clock);
    void createTrackerEndpoints(
        JsonResource& configResource,
        Rtos::ValueMutex<TrackerMap>& sharedTrackers,
        Clock& clock
    );
    void createNetworkEndpoints(JsonResource& configResource);
    void createMeasuringEndpoints(
        JsonResource& configResource,
        std::reference_wrapper<MeasuringUnit>& measuringUnit,
        const Rtos::ValueMutex<MeasurementList>& sharedMeasurements
    );

private:
    RestApi& m_restApi;
};
#pragma once

#include "RestAPI/RestAPI.h"
#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Switch/Switch.h"
#include "Tracker/Tracker.h"
#include "Rtos/ValueMutex/ValueMutex.h"


namespace Api
{
    void createSystemEndpoints(
        RestApi* restApi,
        const Version& firmwareVersion,
        const Version& apiVersion
    ) noexcept;

    void createLoggerEndpoints(
        RestApi* restApi,
        JsonResource* configResource,
        AsyncWebServer* server
    ) noexcept;

    void createSwitchEndpoints(
        RestApi* restApi,
        JsonResource* configResource,
        Switch** switchUnit
    ) noexcept;

    void createMeasuringEndpoints(
        RestApi* restApi,
        JsonResource* configResource,
        MeasuringUnit** measuringUnit,
        Rtos::ValueMutex<MeasurementList>* measurementsValueMutex
    ) noexcept;

    void createClockEndpoints(
        RestApi* restApi,
        JsonResource* configResource,
        Clock** clock
    ) noexcept;

    void createNetworkEndpoints(
        RestApi* restApi,
        JsonResource* configResource
    ) noexcept;

    void createTrackerEndpoints(
        RestApi* restApi,
        JsonResource* configResource,
        Rtos::ValueMutex<TrackerMap>* trackersValueMutex,
        const Clock* clock
    ) noexcept;
};
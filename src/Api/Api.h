#pragma once

#include "RestAPI/RestAPI.h"
#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Switch/Switch.h"
#include "Tracker/Tracker.h"

namespace PM
{
    namespace Api
    {
        void createSystemEndpoints(RestAPI& api);
        void createLoggerEndpoints(RestAPI &api, const JsonResource &configResource, AsyncWebServer &server);
        void createMeasuringEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            std::reference_wrapper<MeasuringUnit>& measuringUnit,
            std::reference_wrapper<Measurement>& measurement
        );
        void createSwitchEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            std::reference_wrapper<Switch>& switchUnit
        );
        void createClockEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            std::reference_wrapper<Clock>& clock
        );
        void createTrackerEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            TrackerMap& trackers,
            Clock& clock
        );
        void createWiFiEndpoints(RestAPI& api, const JsonResource& configResource);
    }
}
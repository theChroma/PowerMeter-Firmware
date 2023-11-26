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
        void createLoggerEndpoints(RestAPI& api, const JsonResource& configResource);
        void createMeasuringEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            MeasuringUnit& measuringUnit,
            Measurement& measurement
        );
        void createSwitchEndpoints(
            RestAPI& api,
            const JsonResource& configResource,
            Switch& switchUnit
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
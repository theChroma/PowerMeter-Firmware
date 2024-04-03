#pragma once

#include "RestAPI/RestAPI.h"
#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Switch/Switch.h"
#include "Tracker/Tracker.h"

namespace PM
{
    class Api
    {
    public:
        Api(RestAPI& api);
        void createSystemEndpoints(const Version& firmwareVersion, const Version& apiVersion);
        void createLoggerEndpoints(JsonResource& configResource, AsyncWebServer &server);
        void createSwitchEndpoints(JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit);
        void createClockEndpoints(JsonResource& configResource, std::reference_wrapper<Clock>& clock);
        void createTrackerEndpoints(JsonResource& configResource, TrackerMap& trackers, Clock& clock);
        void createNetworkEndpoints(JsonResource& configResource);
        void createMeasuringEndpoints(
            JsonResource& configResource,
            std::reference_wrapper<MeasuringUnit>& measuringUnit,
            std::reference_wrapper<Measurement>& measurement
        );

    private:
        RestAPI& m_restApi;
    };
}
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
        void createLoggerEndpoints(const JsonResource &configResource, AsyncWebServer &server);
        void createSwitchEndpoints(const JsonResource& configResource, std::reference_wrapper<Switch>& switchUnit);
        void createClockEndpoints(const JsonResource& configResource, std::reference_wrapper<Clock>& clock);
        void createTrackerEndpoints(const JsonResource& configResource, TrackerMap& trackers, Clock& clock);
        void createWiFiEndpoints(const JsonResource& configResource);
        void createNetworkEndpoints(const JsonResource& configResource);
        void createMeasuringEndpoints(
            const JsonResource& configResource,
            std::reference_wrapper<MeasuringUnit>& measuringUnit,
            std::reference_wrapper<Measurement>& measurement
        );

    private:
        RestAPI& m_restApi;
    };
}
#pragma once

#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Tracker/Tracker.h"
#include "Clock/Clock.h"
#include "Switch/Switch.h"
#include <ESPAsyncWebServer.h>
#include <vector>

namespace PM
{
    namespace Config
    {
        json getLoggerDefault() noexcept;
        void configureLogger(JsonResource& configResource, AsyncWebServer& server);
        void configureLogger(const json& configJson, AsyncWebServer& server);

        json getMeasuringDefault() noexcept;
        std::reference_wrapper<MeasuringUnit> configureMeasuring(JsonResource& configResource);
        std::reference_wrapper<MeasuringUnit> configureMeasuring(const json& configJson);

        json getClockDefault() noexcept;
        std::reference_wrapper<Clock> configureClock(JsonResource& configResource);
        std::reference_wrapper<Clock> configureClock(const json& configJson);

        json getSwitchDefault() noexcept;
        std::reference_wrapper<Switch> configureSwitch(JsonResource& configResource);
        std::reference_wrapper<Switch> configureSwitch(const json& configJson);

        json getTrackersDefault() noexcept;
        TrackerMap configureTrackers(JsonResource& configResource, std::reference_wrapper<Clock> clock);
        TrackerMap configureTrackers(const json& configJson, std::reference_wrapper<Clock> clock);

        json getNetworkDefault();
        void configureNetwork(JsonResource& configResource);
        void configureNetwork(json& configResource);
    }
}
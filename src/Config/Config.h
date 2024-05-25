#pragma once

#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Tracker/Tracker.h"
#include "Clock/Clock.h"
#include "Switch/Switch.h"
#include <ESPAsyncWebServer.h>
#include <vector>


namespace Config
{
    json getLoggerDefault() noexcept;
    void configureLogger(JsonResource* configResource, AsyncWebServer* server);
    void configureLogger(const json& configJson, AsyncWebServer* server);

    json getMeasuringDefault() noexcept;
    MeasuringUnit* configureMeasuring(JsonResource* configResource);
    MeasuringUnit* configureMeasuring(const json& configJson);

    json getClockDefault() noexcept;
    Clock* configureClock(JsonResource* configResource);
    Clock* configureClock(const json& configJson);

    json getSwitchDefault() noexcept;
    Switch* configureSwitch(JsonResource* configResource);
    Switch* configureSwitch(const json& configJson);

    json getTrackersDefault() noexcept;
    TrackerMap configureTrackers(JsonResource* configResource, const Clock* clock);
    TrackerMap configureTrackers(const json& configJson, const Clock* clock);

    json getNetworkDefault() noexcept;
    void configureNetwork(JsonResource* configResource);
    void configureNetwork(json* configJson);
}

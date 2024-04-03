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
        void configureLogger(const JsonResource& configResource, AsyncWebServer& server);
        std::reference_wrapper<MeasuringUnit> configureMeasuringUnit(const JsonResource& configResource);
        std::reference_wrapper<Clock> configureClock(const JsonResource& configResource);
        std::reference_wrapper<Switch> configureSwitch(const JsonResource& configResource);
        TrackerMap configureTrackers(const JsonResource& configResource, std::reference_wrapper<Clock> clock);
        void configureNetwork(JsonResource& configResource);
    }
}
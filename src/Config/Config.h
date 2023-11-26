#pragma once

#include "JsonResource/JsonResource.h"
#include "MeasuringUnit/MeasuringUnit.h"
#include "Tracker/Tracker.h"
#include "Clock/Clock.h"
#include "Switch/Switch.h"
#include <vector>

namespace PM
{  
    namespace Config
    {
        void configureLogger(const JsonResource& configResource);
        MeasuringUnit& configureMeasuringUnit(const JsonResource& configResource);
        Clock& configureClock(const JsonResource& configResource);
        Switch& configureSwitch(const JsonResource& configResource);
        TrackerMap configureTrackers(const JsonResource& configResource, Clock& clock);
        void configureWiFi(const JsonResource& configResource);
    }
}
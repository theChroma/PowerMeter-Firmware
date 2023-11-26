#pragma once

#include "Clock/Clock.h"
#include <json.hpp>

namespace PM
{
    class SimulationClock : public Clock
    {
    public:
        SimulationClock(const json& configJson);

        time_t now() const noexcept override;

    private:
        time_t m_startTimestamp;
        float m_fastForward;
    };
}
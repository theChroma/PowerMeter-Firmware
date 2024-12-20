#pragma once

#include "Clock/Clock.h"

class MockClock : public Clock
{
public:
    MockClock(time_t timestamp = 0) : m_now(timestamp)
    {}

    void setTimestamp(time_t now)
    {
        m_now = now;
    }

    time_t now() const noexcept override
    {
        return m_now;
    }

    void tick(time_t increment_s = 1) noexcept
    {
        m_now += increment_s;
    }
private:
    time_t m_now = 0;
};

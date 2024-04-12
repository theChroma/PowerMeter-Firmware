#pragma once

#include <time.h>

class Clock
{
public:
    virtual time_t now() const noexcept = 0;
    inline virtual ~Clock() noexcept {};
};
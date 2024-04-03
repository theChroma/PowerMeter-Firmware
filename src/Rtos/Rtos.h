#pragma once

#include <mutex>

namespace PM
{
    namespace Rtos
    {
        extern std::mutex trackerAccess;
    }
}
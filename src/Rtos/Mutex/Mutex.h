#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace Rtos
{
    class Mutex
    {
    public:
        Mutex(const char* name);
        virtual ~Mutex();
        void lock(TickType_t timeoutTicks = portMAX_DELAY);
        void unlock();

    private:
        const char* m_name;
        SemaphoreHandle_t m_mutex;
    };
}
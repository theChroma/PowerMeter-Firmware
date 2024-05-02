#ifdef ESP32

#include "Mutex.h"
#include "SourceLocation/SourceLocation.h"
#include <mutex>

using namespace Rtos;

Mutex::Mutex(const char* name) :
    m_name(name),
    m_mutex(xSemaphoreCreateMutex())
{
    if (!m_mutex)
    {
        throw std::runtime_error(SOURCE_LOCATION + "Failed to create mutex \"" + m_name + "\"");
    }
}

Mutex::~Mutex()
{
    vSemaphoreDelete(m_mutex);
}


void Mutex::lock(TickType_t timeoutTicks)
{
    if (!xSemaphoreTake(m_mutex, timeoutTicks))
    {
        throw std::runtime_error(SOURCE_LOCATION + "Locking mutex \"" + m_name + "\" timed out");
    }
}

void Mutex::unlock()
{
    if (!xSemaphoreGive(m_mutex))
    {
        throw std::runtime_error(
            SOURCE_LOCATION +
            "Failed to unlock mutex \"" +
            m_name +
            "\". The mutex might not have been locked before"
        );
    }

}

#endif
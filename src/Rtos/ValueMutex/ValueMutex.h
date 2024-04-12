#pragma once

#include "Rtos/Mutex/Mutex.h"
#include <mutex>

namespace Rtos
{
    template<typename T>
    class ValueMutex
    {
    public:
        ValueMutex() = default;
        ValueMutex(const T& data) :
            m_value(data)
        {}

        ValueMutex& assign(const T& data)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_value = data;
            return *this;
        }

        ValueMutex& operator=(const T& data)
        {
            return assign(data);
        }

        T get() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_value;
        }

        operator T() const
        {
            return get();
        }

    private:
        mutable std::mutex m_mutex;
        T m_value;
    };
}
#pragma once

#include "Rtos/Mutex/Mutex.h"
#include <mutex>

namespace PM
{
    namespace Rtos
    {
        template<typename T>
        class DataMutex
        {
        public:
            DataMutex(const T& data) :
                m_data(data)
            {}

            DataMutex(const DataMutex& other) = delete;

            DataMutex& assign(const T& data)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_data = data;
                return *this;
            }

            DataMutex& operator=(const DataMutex& other) = delete;

            DataMutex& operator=(const T& data)
            {
                return assign();
            }

            T get() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_data;
            }

            operator T() const
            {
                return get();
            }

        private:
            std::mutex m_mutex;
            T m_data;
        };
    }
}
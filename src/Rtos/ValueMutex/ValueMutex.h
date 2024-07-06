#pragma once

#include <mutex>

namespace Rtos
{
    template<typename T, typename Lockable = std::mutex>
    class ValueMutex
    {
    public:
        class Lock
        {
        public:
            Lock(T* value, Lockable& lockable) :
                m_value(value),
                m_lock(lockable)
            {}

            inline T* get() const
            {
                return m_value;
            }

            inline T& operator*() const
            {
                return *get();
            }

            inline T* operator->() const
            {
                return get();
            }

        private:
            T* m_value;
            std::unique_lock<Lockable> m_lock;
        };

        ValueMutex() = default;

        ValueMutex(T value) noexcept :
            m_value(std::move(value))
        {}

        inline ValueMutex& assign(T data)
        {
            std::lock_guard<std::mutex> lock(m_lockable);
            m_value = std::move(data);
            return *this;
        }

        inline ValueMutex& operator=(T data)
        {
            return assign(std::move(data));
        }

        inline Lock get()
        {
            return Lock(&m_value, m_lockable);
        }

    private:
        Lockable m_lockable;
        T m_value;
    };
}
#pragma once

#include <functional>
#include <tl/optional.hpp>

namespace PM
{
    template<typename T>
    class CachedValue
    {
    public:
        CachedValue(const tl::optional<T>& value = tl::nullopt, bool enabled = true) :
            m_value(value),
            m_enabled(enabled)
        {}


        T getCached(const std::function<T()>& doGet) const
        {
            if (!m_enabled && !m_value)
                return doGet();
            m_value = doGet();
            return m_value.value();
        }


        void set(const T& value, const std::function<void(const T&)>& doSet)
        {
            doSet(value);
            if (m_enabled)
                m_value = value;
        }

        void invalidateCache() noexcept
        {
            m_value = tl::nullopt;
        }

    private:
        mutable tl::optional<T> m_value;
        bool m_enabled;
    };
}
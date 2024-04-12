#pragma once

#include <functional>
#include <tl/optional.hpp>

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
        if (!m_enabled)
            return doGet();
        if (!m_value.has_value())
            m_value = doGet();
        return m_value.value();
    }


    void set(const T& value)
    {
        if (m_enabled)
            m_value = value;
    }


    CachedValue& operator=(const T& value)
    {
        set(value);
        return *this;
    }


    void invalidateCache() noexcept
    {
        m_value = tl::nullopt;
    }

private:
    mutable tl::optional<T> m_value;
    bool m_enabled;
};
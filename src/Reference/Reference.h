#pragma once

#include <functional>

template<typename T>
class Reference : public std::reference_wrapper
{
public:
    T* operator->()
    {
        return get();
    }
};
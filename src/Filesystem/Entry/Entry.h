#pragma once

#include <string>

namespace Filesystem
{
    class Entry
    {
    public:
        virtual std::string getPath() const = 0;
        virtual bool exists() const = 0;
        virtual void remove() = 0;
        inline virtual ~Entry() noexcept {};
    };
}
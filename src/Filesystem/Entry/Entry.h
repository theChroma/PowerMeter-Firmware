#pragma once

#include <string>
#include <json.hpp>

namespace Filesystem
{
    class Entry
    {
    public:
        virtual std::string getPath() const = 0;
        virtual std::string getName() const = 0;
        virtual bool exists() const = 0;
        virtual void create() = 0;
        virtual void remove() = 0;
        virtual json toJson() const;
        inline virtual ~Entry() noexcept = default;
    };
}
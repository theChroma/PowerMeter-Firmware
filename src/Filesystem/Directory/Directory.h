#pragma once

#include "Filesystem/Entry/Entry.h"
#include <memory>
#include <vector>

namespace Filesystem
{
    class Directory : public Entry
    {
    public:
        class Open
        {
        public:
            virtual std::vector<std::unique_ptr<Entry>> getEntries() const = 0;
            inline virtual ~Open() noexcept {};
        };
        virtual Directory& create() = 0;
        virtual Directory::Open& open() = 0;
        inline virtual ~Directory() noexcept {};
    };
}
#pragma once

#include "Filesystem/Entry/Entry.h"
#include <memory>
#include <vector>

namespace Filesystem
{
    class Directory : public Entry
    {
    public:
        virtual std::vector<std::unique_ptr<Entry>> getEntries() const = 0;
        virtual void create() = 0;
        inline virtual ~Directory() noexcept = default;
    };
}
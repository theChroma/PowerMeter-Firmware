#pragma once

#include "Filesystem/Entry/Entry.h"
#include <memory>
#include <set>

namespace Filesystem
{
    class Directory : public Entry
    {
    public:

        using Entries = std::set<std::unique_ptr<Entry>, std::function<bool(std::unique_ptr<Entry>, std::unique_ptr<Entry>)>>;
        virtual Entries getEntries() const = 0;
        virtual void create() = 0;
        inline virtual ~Directory() noexcept = default;
    };
}
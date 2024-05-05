#pragma once

#include "Filesystem/Entry/Entry.h"
#include <iostream>
#include <memory>

namespace Filesystem
{
    class File : public Entry
    {
    public:
        virtual std::unique_ptr<std::iostream> open(std::ios::openmode mode = std::ios::in) = 0;
        virtual time_t getLastWriteTimestamp() const = 0;
        inline virtual ~File() noexcept = default;
    };
}
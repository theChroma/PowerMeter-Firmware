#pragma once

#include "Filesystem/Entry/Entry.h"
#include <unique_resource.hpp>
#include <functional>
#include <iostream>

namespace Filesystem
{
    class File : public Entry
    {
    public:
        using  Stream = std_experimental::unique_resource<std::iostream*, std::function<void(std::iostream*)>>;
        virtual Stream open(std::ios::openmode mode = std::ios::in) = 0;
        virtual time_t getLastWriteTimestamp() const = 0;
        json toJson() const override;
        inline virtual ~File() noexcept = default;
    };
}
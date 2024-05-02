#pragma once

#include "Filesystem/Entry/Entry.h"
#include <iostream>
#include <memory>

namespace Filesystem
{
    class File : public Entry
    {
    public:
        class Open
        {
        public:
            virtual std::iostream& getStream() = 0;
            virtual time_t getLastWriteTimestamp() const = 0;
            inline virtual ~Open() noexcept {};
        };

        virtual std::unique_ptr<File::Open> open(std::ios::openmode mode = std::ios::in) = 0;
        virtual File& create() = 0;
        inline virtual ~File() noexcept {};
    };
}
#pragma once

#include "Filesystem/File/File.h"
#include <tl/optional.hpp>
#include <fstream>
#include <string>
#include <FS.h>

namespace Filesystem
{
    class LittleFsFile : public File
    {
    public:
        LittleFsFile(std::string path);
        std::string getPath() const override;
        std::string getName() const override;
        std::unique_ptr<std::iostream> open(std::ios::openmode mode) override;
        time_t getLastWriteTimestamp() const override;
        void create() override;
        bool exists() const override;
        void remove() override;

    private:
        std::string m_path;
    };
}
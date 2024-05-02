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
        class Open : public File::Open
        {
        public:
            Open(std::string path, std::ios::openmode mode = std::ios::in);
            std::iostream& getStream() override;
            time_t getLastWriteTimestamp() const override;

        private:
            std::string m_path;
            std::fstream m_fileStream;
        };

        LittleFsFile(std::string path);
        std::unique_ptr<File::Open> open(std::ios::openmode mode) override;
        File& create() override;
        bool exists() const override;
        void remove() override;

    private:
        std::string m_path;
    };
}
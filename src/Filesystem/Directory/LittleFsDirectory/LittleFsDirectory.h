#pragma once

#include "Filesystem/Directory/Directory.h"

namespace Filesystem
{
    class LittleFsDirectory : public Directory
    {
    public:
        explicit LittleFsDirectory(std::string path) noexcept;
        Entries getEntries() const override;
        std::string getPath() const override;
        std::string getName() const override;
        bool exists() const;
        void create();
        void remove();
    private:
        std::string m_path;
    };
}
#pragma once

#include "Filesystem/Directory/Directory.h"

namespace Filesystem
{
    class LittleFsDirectory : public Directory
    {
    public:
        LittleFsDirectory(std::string path) noexcept;
        std::vector<std::unique_ptr<Entry>> getEntries() const override;
        std::string getPath() const override;
        std::string getName() const override;
        bool exists() const;
        void create();
        void remove();
    private:
        std::string m_path;
    };
}
#pragma once

#include "Filesystem/File/File.h"
#include <sstream>
#include <ctime>

struct MockFile : public Filesystem::File
{
    MockFile(std::string path, std::string name) : path(path), name(name)
    {}

    std::string getPath() const override
    {
        return path;
    }

    std::string getName() const override
    {
        return name;
    }

    bool exists() const override
    {
        return true;
    }

    void create() override
    {}

    void remove() override
    {}

    Stream open(std::ios::openmode mode = std::ios::in) override
    {
        if (mode & std::ios::out)
        {
            lastWriteTimestamp = std::time(nullptr);
            stream.str("");
        }

        return Stream(&stream, [](std::iostream*){});
    }

    time_t getLastWriteTimestamp() const override
    {
        return lastWriteTimestamp;
    }

    std::string path;
    std::string name;
    std::stringstream stream;
    time_t lastWriteTimestamp = 0;
};
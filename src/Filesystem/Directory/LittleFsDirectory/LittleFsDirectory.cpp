#ifdef ESP32

#include "LittleFsDirectory.h"
#include "Filesystem/File/LittleFsFile/LittleFsFile.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include <sstream>
#include <unique_resource.hpp>
#include <dirent.h>
#include <LittleFS.h>

using namespace Filesystem;

LittleFsDirectory::LittleFsDirectory(std::string path) noexcept : m_path(std::move(path))
{}


Directory::Entries LittleFsDirectory::getEntries() const
{
    auto directory = std_experimental::unique_resource<DIR*, std::function<int(DIR*)>>(
        opendir(m_path.c_str()),
        closedir
    );
    if (!directory)
        throw std::runtime_error(SOURCE_LOCATION + "Failed to open directory at \"" + m_path + '"');

    struct dirent* directoryEntry;
    Entries entries([](const std::unique_ptr<Entry>& lhs, const std::unique_ptr<Entry>& rhs){
        return lhs->getPath() == rhs->getPath();
    });
    while (directoryEntry = readdir(directory))
    {
        std::string entryPath = m_path + '/' + directoryEntry->d_name;
        if (directoryEntry->d_type == DT_DIR)
            entries.emplace(new LittleFsDirectory(entryPath));
        else
            entries.emplace(new LittleFsFile(entryPath));
    }
    return entries;
}


std::string LittleFsDirectory::getPath() const
{
    return m_path;
}

std::string LittleFsDirectory::getName() const
{
    return m_path.substr(m_path.rfind('/') + 1);
}

void LittleFsDirectory::create()
{
    std::istringstream pathStream(m_path);
    std::string token;
    std::string currentPath;
    while (std::getline(pathStream, token, '/'))
    {
        if (!token.empty())
        {
            currentPath += '/';
            currentPath += token;
            if (!LittleFS.mkdir(currentPath.c_str()))
                throw std::runtime_error(SOURCE_LOCATION + "Failed to create directory at \"" + m_path + '"');
        }
    }
}


bool LittleFsDirectory::exists() const
{
    return LittleFS.exists(m_path.c_str()) && LittleFS.open(m_path.c_str()).isDirectory();
}


void LittleFsDirectory::remove()
{
    Logger[LogLevel::Debug] << "Trying to remove dir " << m_path << std::endl;
    while (true)
    {
        Entries entries = getEntries();
        if (entries.empty())
            break;
        for (const auto& entry : entries)
            entry->remove();
    }
    if (!LittleFS.rmdir(m_path.c_str()))
        throw std::runtime_error(SOURCE_LOCATION + "Failed to remove directory at \"" + m_path + '"');
}

#endif
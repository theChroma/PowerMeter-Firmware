#ifdef ESP32

#include "LittleFsFile.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include <LittleFS.h>

using namespace Filesystem;

LittleFsFile::LittleFsFile(std::string path) : m_path(std::move(path))
{}


std::string LittleFsFile::getPath() const
{
    return m_path;
}


std::string Filesystem::LittleFsFile::getName() const
{
    return m_path.substr(m_path.rfind('/') + 1);
}


Filesystem::File::Stream LittleFsFile::open(std::ios::openmode mode)
{
    if ((mode & std::ios::out) && !exists())
        create();

    m_fileStream.open(m_path, mode);
    if (!m_fileStream.good())
        throw std::runtime_error(SOURCE_LOCATION + "Failed to open file at \"" + m_path + '"');
    return File::Stream(&m_fileStream, [this](std::iostream*){
        m_fileStream.close();
    });
}


time_t LittleFsFile::getLastWriteTimestamp() const
{
    fs::File file = LittleFS.open(m_path.c_str());
    if (!file)
        throw std::runtime_error(SOURCE_LOCATION + "Failed to open file at \"" + m_path + '"');
    return file.getLastWrite();
}


void LittleFsFile::create()
{
    LittleFS.open(m_path.c_str(), "w", true);
}


bool LittleFsFile::exists() const
{
    return LittleFS.exists(m_path.c_str()) && !LittleFS.open(m_path.c_str()).isDirectory();
}


void LittleFsFile::remove()
{
    if (!LittleFS.remove(m_path.c_str()))
        throw std::runtime_error(SOURCE_LOCATION + "Failed to remove file at \"" + m_path + '"');
}

#endif
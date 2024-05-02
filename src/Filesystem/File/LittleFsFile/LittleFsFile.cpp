#ifdef ESP32

#include "LittleFsFile.h"
#include "SourceLocation/SourceLocation.h"
#include <LittleFS.h>

using namespace Filesystem;

LittleFsFile::LittleFsFile(std::string path) : m_path(std::move(path))
{}


std::unique_ptr<Filesystem::File::Open> LittleFsFile::open(std::ios::openmode mode)
{
    return std::unique_ptr<Open>(new Open(m_path, mode));
}


Filesystem::File& LittleFsFile::create()
{
    LittleFS.open(m_path.c_str(), "w", true);
    return *this;
}


bool LittleFsFile::exists() const
{
    return LittleFS.exists(m_path.c_str());
}


void LittleFsFile::remove()
{
    if (!LittleFS.remove(m_path.c_str()))
        throw std::runtime_error(SOURCE_LOCATION + "Failed to remove file at \"" + m_path + '"');
}


LittleFsFile::Open::Open(std::string path, std::ios::openmode mode) :
    m_path(std::move(path))
{
    if (mode & std::ios::out)
        LittleFS.open(m_path.c_str(), "w", true);

    m_fileStream.open(m_path, mode);
    if (!m_fileStream.good())
        throw std::runtime_error(SOURCE_LOCATION + "Failed to open file at \"" + m_path + '"');
}


std::iostream& LittleFsFile::Open::getStream()
{
    return m_fileStream;
}


time_t Filesystem::LittleFsFile::Open::getLastWriteTimestamp() const
{
    fs::File file = LittleFS.open(m_path.c_str());
    if (!file)
        throw std::runtime_error(SOURCE_LOCATION + '"' + m_path + "\" is not a valid filepath");
    return file.getLastWrite();
}

#endif
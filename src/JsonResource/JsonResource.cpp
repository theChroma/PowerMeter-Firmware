#include "JsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include <fstream>
#include <exception>

#ifdef ESP32
#include <LittleFS.h>
#include <regex>
#endif

using PM::JsonResource;

JsonResource::JsonResource(const std::string& filePath, bool useCaching) noexcept :
    m_filePath(filePath),
    m_cachedData(CachedValue<json>(tl::nullopt, useCaching))
{}


void JsonResource::serialize(const json& data)
{
    try
    {
#ifdef ESP32
        if (!m_directoryExists)
        {
            std::regex directoryRegex("\\/[^\\/]+(?=\\/)");
            std::string currentDirectory = "";
            for(std::sregex_iterator i(m_filePath.begin(), m_filePath.end(), directoryRegex); i != std::sregex_iterator(); i++)
            {
                std::smatch match = *i;
                currentDirectory += match.str();
                LittleFS.mkdir(currentDirectory.c_str());
                m_directoryExists = true;
            }
        }
#endif

        std::ofstream file;
        file.open(m_filePath);
        file << data.dump(1, '\t') << std::flush;
        m_cachedData = data;
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to serialize \"" + data.dump() + "\" to \"" + std::string(*this) + "\"");
        throw;
    }
}


json JsonResource::deserialize() const
{
    try
    {
        return m_cachedData.getCached([this]{
            std::ifstream file(m_filePath);
            if(!file.good())
                throw std::runtime_error('"' + m_filePath + "\" is not a valid filepath");
            return json::parse(file);
        });
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to deserialize \"" + std::string(*this) + "\"");
        throw;
    }
}


json JsonResource::deserializeOr(const json& defaultJson) const
{
    try
    {
        return deserialize();
    }
    catch (...)
    {
        ExceptionTrace::clear();
        return defaultJson;
    }
}


json JsonResource::deserializeOrGet(const std::function<json()>& getDefaultJson) const
{
    try
    {
        return deserialize();
    }
    catch (...)
    {
        ExceptionTrace::clear();
        return getDefaultJson();
    }
}


void JsonResource::erase()
{
    try
    {
        m_cachedData.invalidateCache();
        std::string filePath = m_filePath;
        JsonResource fileResource(filePath);

        // Erase at file level
#ifdef ESP32
        auto popBackPath = [](std::string& path, char seperator = '/'){
            size_t seperatorIndex = path.rfind('/');
            if(seperatorIndex == std::string::npos)
            path = "";
            path.erase(seperatorIndex);
        };

        LittleFS.remove(m_filePath.c_str());
        popBackPath(filePath);
        while(!filePath.empty())
        {
            LittleFS.rmdir(filePath.c_str());
            popBackPath(filePath);
        }
#endif
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to erase\"" + std::string(*this) + "\"");
        throw;
    }
}


void JsonResource::setFilePath(const std::string& filePath) noexcept
{
    m_filePath = filePath;
}


std::string JsonResource::getFilePath() const noexcept
{
    return m_filePath;
}


JsonResource::operator std::string() const noexcept
{
    return getFilePath();
}


namespace PM
{
    std::ostream& operator<<(std::ostream& os, const JsonResource& JsonResource) noexcept
    {
        os << static_cast<std::string>(JsonResource);
        return os;
    }
}
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

JsonResource::JsonResource(const std::string& filePath, const json::json_pointer& jsonPointer, bool useCaching) noexcept :
    m_filePath(filePath),
    m_jsonPointer(jsonPointer),
    m_cachedData(CachedValue<json>(tl::nullopt, useCaching))
{}


JsonResource::JsonResource(const std::string& uri, bool useCaching) :
    m_cachedData(CachedValue<json>(tl::nullopt, useCaching))
{
    try
    {
        size_t seperatorIndex = uri.find('#');
        if(seperatorIndex != std::string::npos)
        {
            m_filePath = uri.substr(0, seperatorIndex);
            m_jsonPointer = json::json_pointer(uri.substr(seperatorIndex + 1, uri.size()));
        }
        else
        {
            m_filePath = uri;
        }
    }
    catch(...)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to construct from \"" << uri << "\"";
        ExceptionTrace::trace(errorMessage.str());
        throw;
    }
}


void JsonResource::serialize(const json& data)
{
    try
    {
        m_cachedData.set(data, [this](const json& data){
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
            if(m_jsonPointer.empty())
            {
                file.open(m_filePath);
                file << data.dump(1, '\t') << std::flush;
            }
            else
            {
                json fileData;
                try
                {
                    fileData = JsonResource(m_filePath, json::json_pointer()).deserialize();
                }
                catch(json::exception)
                {
                    ExceptionTrace::clear();
                }
                catch(std::runtime_error)
                {
                    ExceptionTrace::clear();
                }
                file.open(m_filePath);
                fileData[m_jsonPointer].merge_patch(data);
                file << fileData.dump(1, '\t') << std::flush;

            }
        });
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
            return json::parse(file).at(m_jsonPointer);
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
        // Logger[LogLevel::Debug] << '"' << *this << "\" using default value" << std::endl;
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

        // Erase at json level
        if(!m_jsonPointer.empty())
        {
            json data = fileResource.deserializeOr(json());
            json patch;
            patch["/0/op"_json_pointer] = "remove";
            patch["/0/path"_json_pointer] = m_jsonPointer.to_string();
            try
            {
                data.patch_inplace(patch);
            }
            catch(json::out_of_range)
            {
                ExceptionTrace::clear();
            }
            json flattenedData = data.flatten();
            for(const auto& flattenedDataElement : flattenedData)
            {
                if(!flattenedDataElement.empty())
                {
                    json data = flattenedData.unflatten();
                    fileResource.serialize(data);
                    return;
                }
            }
        }

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


void JsonResource::setJsonPointer(const json::json_pointer& jsonPointer) noexcept
{
    m_jsonPointer = jsonPointer;
}


void JsonResource::setFilePath(const std::string& filePath) noexcept
{
    m_filePath = filePath;
}


std::string JsonResource::getFilePath() const noexcept
{
    return m_filePath;
}


json::json_pointer JsonResource::getJsonPointer() const noexcept
{
    return m_jsonPointer;
}


JsonResource::operator std::string() const noexcept
{
    std::string uri = m_filePath;
    if(!m_jsonPointer.empty())
    {
        uri += '#';
        uri += m_jsonPointer.to_string();
    }
    return uri;
}


JsonResource& JsonResource::operator/=(const json::json_pointer& jsonPointer) noexcept
{
    m_jsonPointer /= jsonPointer;
    return *this;
}

namespace PM
{
    JsonResource operator/(JsonResource lhs, const json::json_pointer& rhs) noexcept
    {
        lhs /= rhs;
        return lhs;
    }

    std::ostream& operator<<(std::ostream& os, const JsonResource& JsonResource) noexcept
    {
        os << static_cast<std::string>(JsonResource);
        return os;
    }
}
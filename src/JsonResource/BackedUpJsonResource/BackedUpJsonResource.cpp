#include "BackedUpJsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include "ScopeProfiler/ScopeProfiler.h"
#include <string>
#include <LittleFS.h>
#include <sys/time.h>
#include <fstream>
#include <map>

using namespace PM;

namespace
{
    std::unordered_map<std::string, bool> lastModifiedResourceIndices;


    time_t getLastModificationTime(const JsonResource& resource)
    {
        File file = LittleFS.open(resource.getFilePath().c_str());
        time_t lastModificationTime = file ? file.getLastWrite() : 0;
        file.close();
        return lastModificationTime;
    }


    void correctLastModificationTime(const JsonResource& resource)
    {
        time_t now = 0;
        time(&now);
        time_t lastModificationTime = getLastModificationTime(resource);
        // If last modification time is in future...
        if (lastModificationTime > now)
        {
            // Set last modification time to now
            std::ofstream file(resource.getFilePath(), std::ios::app);
        }
    }
}


BackedUpJsonResource::BackedUpJsonResource(const std::string& filePath, const json::json_pointer& jsonPointer, bool useCaching) noexcept :
    JsonResource(filePath, jsonPointer, useCaching),
    m_resources(getResources())
{
    correctLastModificationTime(m_resources[0]);
    correctLastModificationTime(m_resources[1]);
}


BackedUpJsonResource::BackedUpJsonResource(const std::string& uri, bool useCaching) :
    JsonResource(uri, useCaching),
    m_resources(getResources())
{
    correctLastModificationTime(m_resources[0]);
    correctLastModificationTime(m_resources[1]);
}


json BackedUpJsonResource::deserialize() const
{
    try
    {
        bool preferredResourceIndex = getLastModifiedResourceIndex();
        if (getFilePath() == "/Trackers/3600_60/timestamps.json" || getFilePath() == "/Trackers/604800_7/timestamps.json")
        {
            Logger[LogLevel::Debug] << '"' << *this << "\" deserailized from \"" << m_resources[preferredResourceIndex] << '"' << std::endl;
        }
        return m_resources[preferredResourceIndex].deserializeOrGet([this, preferredResourceIndex]{
            return m_resources[!preferredResourceIndex].deserializeOrGet([this]{
                return JsonResource::deserialize();
            });
        });
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to deserialize \"" + std::string(*this) + "\"");
        throw;
    }
}

void BackedUpJsonResource::serialize(const json &data)
{
    try
    {
        bool resourceIndex = !getLastModifiedResourceIndex();
        if (getFilePath() == "/Trackers/3600_60/timestamps.json" || getFilePath() == "/Trackers/604800_7/timestamps.json")
        {
            Logger[LogLevel::Debug] << '"' << *this << "\" serailized to \"" << m_resources[resourceIndex] << '"' << std::endl;
        }
        m_resources[resourceIndex].serialize(data);
        lastModifiedResourceIndices[getFilePath()] = resourceIndex;
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to serialize \"" + std::string(*this) + "\"");
        throw;
    }
}


void BackedUpJsonResource::erase()
{
    try
    {
        m_resources[0].erase();
        m_resources[1].erase();
        JsonResource::erase();
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to erase \"" + std::string(*this) + "\"");
        throw;
    }
}


std::array<JsonResource, 2> BackedUpJsonResource::getResources() const
{
    std::string filePath = getFilePath();
    size_t fileExtensionIndex = filePath.rfind('.');
    if (fileExtensionIndex == std::string::npos)
        fileExtensionIndex = filePath.length() - 1;

    return {
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".a"), getJsonPointer(), false),
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".b"), getJsonPointer(), false),
    };
}


bool BackedUpJsonResource::getLastModifiedResourceIndex() const
{
    try
    {
        return lastModifiedResourceIndices.at(getFilePath());
    }
    catch(std::out_of_range)
    {
        ExceptionTrace::clear();
        time_t lastModificationTimes[2] = {
            getLastModificationTime(m_resources[0]),
            getLastModificationTime(m_resources[1]),
        };
        bool lastModifiedResourceIndex = lastModificationTimes[0] <= lastModificationTimes[1];
        lastModifiedResourceIndices[getFilePath()] = lastModifiedResourceIndex;
        return lastModifiedResourceIndex;
    }
}

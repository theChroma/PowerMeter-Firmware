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

namespace
{
    time_t readLastModificationTime(const JsonResource& resource)
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
        time_t lastModificationTime = readLastModificationTime(resource);
        // If last modification time is in future...
        if (lastModificationTime > now)
        {
            // Set last modification time to now
            std::ofstream file(resource.getFilePath(), std::ios::app);
        }
    }
}


BackedUpJsonResource::BackedUpJsonResource(const std::string& filePath, bool useCaching) noexcept :
    JsonResource(filePath, useCaching),
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
        m_resources[resourceIndex].serialize(data);
        m_cachedLastModifiedResourceIndex = resourceIndex;
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
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".a"), false),
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".b"), false),
    };
}


bool BackedUpJsonResource::getLastModifiedResourceIndex() const
{
    return m_cachedLastModifiedResourceIndex.getCached([this]{
        time_t lastModificationTimes[2] = {
            readLastModificationTime(m_resources[0]),
            readLastModificationTime(m_resources[1]),
        };
        bool lastModifiedResourceIndex = lastModificationTimes[0] <= lastModificationTimes[1];
        return lastModifiedResourceIndex;
    });
}

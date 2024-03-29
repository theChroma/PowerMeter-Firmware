#include "BackedUpJsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "Logger/Logger.h"
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
        Logger[LogLevel::Debug] << '"' << resource.getFilePath() << "\" was modified at " << lastModificationTime << std::endl;
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
            Logger[LogLevel::Info]
                << "Setting last modification time of \""
                << resource << "\" from "
                << lastModificationTime
                << " to "
                << now
                << std::endl;

            std::ofstream file(resource.getFilePath(), std::ios::app);
        }
    }
}


BackedUpJsonResource::BackedUpJsonResource(const std::string& filePath, const json::json_pointer& jsonPointer) noexcept :
    JsonResource(filePath, jsonPointer),
    m_resources(getResources())
{
    correctLastModificationTime(m_resources[0]);
    correctLastModificationTime(m_resources[1]);
}


BackedUpJsonResource::BackedUpJsonResource(const std::string& uri) :
    JsonResource(uri),
    m_resources(getResources())
{
    correctLastModificationTime(m_resources[0]);
    correctLastModificationTime(m_resources[1]);
}


json BackedUpJsonResource::deserialize() const
{
    bool preferredResourceIndex = getLastModifiedResourceIndex();
    try
    {
        try
        {
            return m_resources[preferredResourceIndex].deserialize();
        }
        catch (...)
        {
            ExceptionTrace::clear();
            Logger[LogLevel::Warning]
                << "Failed to deserialize \""
                << m_resources[preferredResourceIndex]
                << "\". Using \""
                << m_resources[!preferredResourceIndex]
                << "\" as fallback."
                << std::endl;
            return m_resources[!preferredResourceIndex].deserialize();
        }
    }
    catch(...)
    {
        ExceptionTrace::clear();
        Logger[LogLevel::Warning]
                << "Failed to deserialize \""
                << m_resources[!preferredResourceIndex]
                << "\". Using \""
                << *this
                << "\" as fallback."
                << std::endl;
        return JsonResource::deserialize();
    }
}

void BackedUpJsonResource::serialize(const json &data) const
{
    bool resourceIndex = !getLastModifiedResourceIndex();
    m_resources[resourceIndex].serialize(data);
}


void BackedUpJsonResource::erase() const
{
    m_resources[0].erase();
    m_resources[1].erase();
    JsonResource::erase();
}


std::array<JsonResource, 2> BackedUpJsonResource::getResources() const
{
    std::string filePath = getFilePath();
    size_t fileExtensionIndex = filePath.rfind('.');
    if (fileExtensionIndex == std::string::npos)
        fileExtensionIndex = filePath.length() - 1;

    return {
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".a"), getJsonPointer()),
        JsonResource(std::string(filePath).insert(fileExtensionIndex, ".b"), getJsonPointer()),
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

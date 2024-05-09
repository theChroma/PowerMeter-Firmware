#ifdef ESP32

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


BackedUpJsonResource::BackedUpJsonResource(BasicJsonResource resourceA, BasicJsonResource resourceB) noexcept :
    m_resourceA(std::move(resourceA)),
    m_resourceB(std::move(resourceB))
{
    time_t lastWriteTimestampResourceA = resourceA.getFile().getLastWriteTimestamp();
    time_t lastWriteTimestampResourceB = resourceB.getFile().getLastWriteTimestamp();

    m_preferredResourceForRead = lastWriteTimestampResourceA > lastWriteTimestampResourceB ? resourceA : resourceB;
    m_preferredResourceForWrite = lastWriteTimestampResourceA > lastWriteTimestampResourceB ? resourceB : resourceA;
}


json BackedUpJsonResource::deserialize() const
{
    try
    {
        return m_preferredResourceForRead->deserializeOrGet([this]{
            return m_preferredResourceForWrite->deserialize();
        });
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to deserialize");
        throw;
    }
}

void BackedUpJsonResource::serialize(const json &data)
{
    try
    {
        m_preferredResourceForWrite.get().serialize(data);
        std::swap(m_preferredResourceForRead, m_preferredResourceForWrite);
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to serialize");
        throw;
    }
}


void BackedUpJsonResource::remove()
{
    try
    {
        m_preferredResourceForRead->remove();
        m_preferredResourceForWrite->remove();
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to remove");
        throw;
    }
}

#endif
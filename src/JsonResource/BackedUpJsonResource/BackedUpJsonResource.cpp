#include "BackedUpJsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include <sys/time.h>


BackedUpJsonResource::BackedUpJsonResource(BasicJsonResource resourceA, BasicJsonResource resourceB) :
    m_resourceA(std::move(resourceA)),
    m_resourceB(std::move(resourceB)),
    m_preferredResourceForRead(&m_resourceA),
    m_preferredResourceForWrite(&m_resourceB)
{
    time_t lastWriteTimestampResourceA = 0;
    time_t lastWriteTimestampResourceB = 0;
    try
    {
        lastWriteTimestampResourceA = m_resourceA.getFile().getLastWriteTimestamp();
        lastWriteTimestampResourceB = m_resourceB.getFile().getLastWriteTimestamp();
    }
    catch(...)
    {}


    m_preferredResourceForRead = lastWriteTimestampResourceA > lastWriteTimestampResourceB ? &m_resourceA : &m_resourceB;
    m_preferredResourceForWrite = lastWriteTimestampResourceA > lastWriteTimestampResourceB ? &m_resourceB : &m_resourceA;
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
        m_preferredResourceForWrite->serialize(data);
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
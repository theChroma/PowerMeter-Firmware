#include "BasicJsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"


BasicJsonResource::BasicJsonResource(std::unique_ptr<Filesystem::File> file, bool useCaching) noexcept :
    m_file(std::move(file)),
    m_cachedData(CachedValue<json>(tl::nullopt, useCaching))
{}


json BasicJsonResource::deserialize() const
{
    try
    {
        return m_cachedData.getCached([this]{
            return json::parse(*m_file->open());
        });
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to deserialize \"" + m_file->getPath() + "\"");
        throw;
    }
}


void BasicJsonResource::serialize(const json& data)
{
    try
    {
        *m_file->open(std::ios::out) << data.dump(1, '\t') << std::flush;
        m_cachedData = data;
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to serialize \"" + data.dump() + "\" to \"" + m_file->getPath() + "\"");
        throw;
    }
}


void BasicJsonResource::remove()
{
    try
    {
        m_cachedData.invalidateCache();
        m_file->remove();
    }
    catch(...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to erase \"" + m_file->getPath() + "\"");
        throw;
    }
}


Filesystem::File& BasicJsonResource::getFile()
{
    return *m_file;
}

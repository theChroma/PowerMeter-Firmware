#include "JsonResource.h"
#include "ExceptionTrace/ExceptionTrace.h"


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

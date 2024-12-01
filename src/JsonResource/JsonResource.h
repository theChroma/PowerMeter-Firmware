#pragma once

#include <Filesystem/File/File.h>
#include <functional>
#include <json.hpp>

class JsonResource
{
public:
    virtual json deserialize() = 0;
    virtual void serialize(const json& data) = 0;
    virtual void remove() = 0;
    json deserializeOr(const json& defaultJson);
    json deserializeOrGet(const std::function<json()>& getDefaultJson);
    inline virtual ~JsonResource() noexcept = default;
};
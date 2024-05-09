#pragma once

#include <Filesystem/File/File.h>
#include <functional>
#include <json.hpp>

class JsonResource
{
public:
    virtual json deserialize() const = 0;
    virtual void serialize(const json& data) = 0;
    virtual void remove() = 0;
    json deserializeOr(const json& defaultJson) const;
    json deserializeOrGet(const std::function<json()>& getDefaultJson) const;
};
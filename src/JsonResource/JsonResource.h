#pragma once

#include "CachedValue/CachedValue.h"
#include <json.hpp>
#include <functional>
#include <tl/optional.hpp>

class JsonResource
{
public:
    JsonResource(const std::string& filePath, bool useCaching = true) noexcept;

    virtual json deserialize() const;
    virtual json deserializeOr(const json& defaultJson) const;
    virtual json deserializeOrGet(const std::function<json()>& getDefaultJson) const;
    virtual void serialize(const json& data);
    virtual void erase();

    void setFilePath(const std::string& path) noexcept;
    std::string getFilePath() const noexcept;

    operator std::string() const noexcept;

protected:
    CachedValue<json> m_cachedData;

private:
    mutable bool m_directoryExists = false;
    std::string m_filePath;
};

std::ostream& operator<<(std::ostream& os, const JsonResource& jsonResource) noexcept;
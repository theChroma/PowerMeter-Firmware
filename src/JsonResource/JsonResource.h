#pragma once

#include "CachedValue/CachedValue.h"
#include <json.hpp>
#include <functional>
#include <tl/optional.hpp>

namespace PM
{
    class JsonResource
    {
    public:
        JsonResource(const std::string& path, const json::json_pointer& jsonPointer, bool useCaching = true) noexcept;
        JsonResource(const std::string& uri, bool useCaching = true);

        virtual json deserialize() const;
        virtual json deserializeOr(const json& defaultJson) const;
        virtual json deserializeOrGet(const std::function<json()>& getDefaultJson) const;
        virtual void serialize(const json& data);
        virtual void erase();

        void setJsonPointer(const json::json_pointer& jsonPointer) noexcept;
        void setFilePath(const std::string& path) noexcept;
        json::json_pointer getJsonPointer() const noexcept;
        std::string getFilePath() const noexcept;

        operator std::string() const noexcept;
        JsonResource& operator/=(const json::json_pointer& jsonPointer) noexcept;

    protected:
        CachedValue<json> m_cachedData;

    private:
        mutable bool m_directoryExists = false;
        std::string m_filePath;
        json::json_pointer m_jsonPointer;
    };

    JsonResource operator/(JsonResource lhs, const json::json_pointer& rhs) noexcept;
    std::ostream& operator<<(std::ostream& os, const JsonResource& jsonResource) noexcept;
}
#pragma once

#include <json.hpp>

namespace PM
{
    class JsonResource
    {
    public:
        JsonResource() noexcept {};
        JsonResource(const std::string& path, const json::json_pointer& jsonPointer) noexcept;
        JsonResource(const std::string& uri);

        json deserialize() const;
        void serialize(const json& data) const;
        void erase() const;

        void setJsonPointer(const json::json_pointer& jsonPointer) noexcept;
        void setFilePath(const std::string& path) noexcept;
        json::json_pointer getJsonPointer() const noexcept;
        std::string getFilePath() const noexcept;

        operator std::string() const noexcept;
        JsonResource& operator/=(const json::json_pointer& jsonPointer) noexcept;

    private:
        std::string m_filePath;
        json::json_pointer m_jsonPointer;
    };

    JsonResource operator/(JsonResource lhs, const json::json_pointer& rhs) noexcept;
    std::ostream& operator<<(std::ostream& os, const JsonResource& jsonResource) noexcept;
}
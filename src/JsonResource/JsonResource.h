#pragma once

#include <json.hpp>

namespace PM
{
    class JsonResource
    {
    public:
        JsonResource(const std::string& path, const json::json_pointer& jsonPointer) noexcept;
        JsonResource(const std::string& uri);

        virtual json deserialize() const;
        virtual void serialize(const json& data) const;
        virtual void erase() const;

        virtual void setJsonPointer(const json::json_pointer& jsonPointer) noexcept;
        virtual void setFilePath(const std::string& path) noexcept;
        virtual json::json_pointer getJsonPointer() const noexcept;
        virtual std::string getFilePath() const noexcept;

        virtual operator std::string() const noexcept;
        virtual JsonResource& operator/=(const json::json_pointer& jsonPointer) noexcept;

    private:
        std::string m_filePath;
        json::json_pointer m_jsonPointer;
    };

    JsonResource operator/(JsonResource lhs, const json::json_pointer& rhs) noexcept;
    std::ostream& operator<<(std::ostream& os, const JsonResource& jsonResource) noexcept;
}
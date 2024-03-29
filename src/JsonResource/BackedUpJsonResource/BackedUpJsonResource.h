#pragma once

#include "JsonResource/JsonResource.h"
#include <array>


namespace PM
{
    class BackedUpJsonResource : public JsonResource
    {
    public:
        BackedUpJsonResource(const std::string& path, const json::json_pointer& jsonPointer) noexcept;
        BackedUpJsonResource(const std::string& uri);

        json deserialize() const override;
        void serialize(const json& data) const override;
        void erase() const override;

    private:
        std::array<JsonResource, 2> getResources() const;
        bool getLastModifiedResourceIndex() const;

        std::array<JsonResource, 2> m_resources;
    };
}
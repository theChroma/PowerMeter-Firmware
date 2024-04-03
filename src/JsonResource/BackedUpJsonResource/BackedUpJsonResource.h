#pragma once

#include "JsonResource/JsonResource.h"
#include <array>


namespace PM
{
    class BackedUpJsonResource : public JsonResource
    {
    public:
        BackedUpJsonResource(const std::string& path, const json::json_pointer& jsonPointer, bool useCaching = true) noexcept;
        BackedUpJsonResource(const std::string& uri, bool useCaching = true);

        json deserialize() const override;
        void serialize(const json& data) override;
        void erase() override;

    private:
        std::array<JsonResource, 2> getResources() const;
        bool getLastModifiedResourceIndex() const;

        std::array<JsonResource, 2> m_resources;
    };
}
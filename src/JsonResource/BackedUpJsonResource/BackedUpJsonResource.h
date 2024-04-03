#pragma once

#include "JsonResource/JsonResource.h"
#include "CachedValue/CachedValue.h"
#include <array>


namespace PM
{
    class BackedUpJsonResource : public JsonResource
    {
    public:
        BackedUpJsonResource(const std::string& path, bool useCaching = true) noexcept;

        json deserialize() const override;
        void serialize(const json& data) override;
        void erase() override;

    private:
        std::array<JsonResource, 2> getResources() const;
        bool getLastModifiedResourceIndex() const;

        CachedValue<bool> m_cachedLastModifiedResourceIndex;
        std::array<JsonResource, 2> m_resources;
    };
}
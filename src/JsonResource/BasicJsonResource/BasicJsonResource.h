#pragma once

#include "JsonResource/JsonResource.h"
#include "Filesystem/File/File.h"
#include "CachedValue/CachedValue.h"
#include <memory>

class BasicJsonResource : public JsonResource
{
public:
    BasicJsonResource(std::unique_ptr<Filesystem::File> file, bool useCaching = true) noexcept;

    json deserialize() const override;
    void serialize(const json& data) override;
    void remove() override;
    Filesystem::File& getFile();

private:
    std::unique_ptr<Filesystem::File> m_file;
    CachedValue<json> m_cachedData;
};

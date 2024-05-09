#pragma once

#include "JsonResource/BasicJsonResource/BasicJsonResource.h"

class BackedUpJsonResource : public JsonResource
{
public:
    BackedUpJsonResource(BasicJsonResource resourceA, BasicJsonResource resourceB) noexcept;

    json deserialize() const override;
    void serialize(const json& data) override;
    void remove() override;

private:
    BasicJsonResource m_resourceA;
    BasicJsonResource m_resourceB;
    std::reference_wrapper<BasicJsonResource> m_preferredResourceForRead;
    std::reference_wrapper<BasicJsonResource> m_preferredResourceForWrite;
};
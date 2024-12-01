#pragma once

#include "JsonResource/BasicJsonResource/BasicJsonResource.h"

class BackedUpJsonResource : public JsonResource
{
public:
    BackedUpJsonResource(BasicJsonResource resourceA, BasicJsonResource resourceB);

    json deserialize() override;
    void serialize(const json& data) override;
    void remove() override;

private:
    BasicJsonResource m_resourceA;
    BasicJsonResource m_resourceB;
    BasicJsonResource* m_preferredResourceForRead;
    BasicJsonResource* m_preferredResourceForWrite;
};
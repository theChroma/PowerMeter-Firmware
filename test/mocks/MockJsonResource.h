#pragma once

#include "JsonResource/JsonResource.h"

class MockJsonResource : public JsonResource
{
public:
    json deserialize() const override
    {
        return m_data;
    }

    void serialize(const json& data) override
    {
        m_data = data;
    }

    void remove() override
    {}


private:
    json m_data;
};
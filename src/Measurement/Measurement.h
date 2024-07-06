#pragma once

#include <vector>
#include <json.hpp>


struct Measurement
{
    json toJson() const;
    const char* name;
    float value;
    const char* unit;
    uint8_t fractionDigits;
};

using MeasurementList = std::vector<Measurement>;
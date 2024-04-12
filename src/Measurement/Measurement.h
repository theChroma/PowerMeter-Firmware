#pragma once

#include <vector>
#include <json.hpp>


struct Measurement
{
    json toJson() const;
    const char* name;
    float value;
    const char* unit;
};

using MeasurementList = std::vector<Measurement>;
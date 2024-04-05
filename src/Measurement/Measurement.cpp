#include "Measurement.h"

using namespace PM;

json Measurement::toJson() const
{
    return {
        {"name", name},
        {"value", value},
        {"unit", unit},
    };
}
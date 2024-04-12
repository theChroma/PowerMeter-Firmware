#include "Measurement.h"


json Measurement::toJson() const
{
    return {
        {"name", name},
        {"value", value},
        {"unit", unit},
    };
}
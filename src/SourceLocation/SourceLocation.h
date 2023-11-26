#pragma once

#include <sstream>

#define SOURCE_LOCATION                                     \
static_cast<std::ostringstream&&>(                          \
    std::ostringstream()                                    \
        << __FILE__                                         \
        << ":" << __LINE__                                  \
        << " in '" << __PRETTY_FUNCTION__ << "': "          \
).str()


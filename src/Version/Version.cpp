#include "Version.h"
#include <cstdio>
#include <cinttypes>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"

#include <string.h>

using namespace PM;

Version::Version(uint16_t major, uint16_t minor, uint16_t patch) noexcept :
    major(major),
    minor(minor),
    patch(patch)
{}


Version::Version(const std::string& version)
{
    if (sscanf(version.c_str(), "%" SCNu16 ".%" SCNu16 ".%" SCNu16, &major, &minor, &patch) != 3)
    {
        std::stringstream errorMessage;
        errorMessage << SOURCE_LOCATION << "Failed to parse \"" << version << "\" as Version";
        throw std::runtime_error(errorMessage.str());
    }
}

bool Version::operator==(const Version &other) const noexcept
{
    return major == other.major && minor == other.minor && patch == other.patch;
}


bool Version::operator!=(const Version &other) const noexcept
{
    return !(*this == other);
}


bool Version::operator>(const Version &other) const noexcept
{
    return major > other.major || minor > other.minor || patch > other.patch;
}


bool Version::operator>=(const Version &other) const noexcept
{
    return *this > other || *this == other;
}


bool Version::operator<(const Version &other) const noexcept
{
    return major < other.major || minor < other.minor || patch < other.patch;
}


bool Version::operator<=(const Version &other) const noexcept
{
    return *this < other || *this == other;
}


Version::operator std::string() const noexcept
{
    std::stringstream version;
    version << major << '.' << minor << '.' << patch;
    return version.str();
}

namespace PM
{
    std::ostream& operator<<(std::ostream& os, const Version& version) noexcept
    {
        os << static_cast<std::string>(version);
        return os;
    }
}
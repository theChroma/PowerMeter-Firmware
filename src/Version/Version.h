#pragma once

#include <string>
#include <iostream>
#include <cstdint>

struct Version
{
    Version(uint16_t major, uint16_t minor, uint16_t patch) noexcept;
    Version(const std::string& version);

    operator std::string() const noexcept;
    bool operator==(const Version& other) const noexcept;
    bool operator!=(const Version& other) const noexcept;
    bool operator>(const Version& other) const noexcept;
    bool operator>=(const Version& other) const noexcept;
    bool operator<(const Version& other) const noexcept;
    bool operator<=(const Version& other) const noexcept;

    uint16_t major;
    uint16_t minor;
    uint16_t patch;
};

std::ostream& operator<<(std::ostream& os, const Version& version) noexcept;
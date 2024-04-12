#include "LogLevel.h"
#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace
{
    std::unordered_map<LogLevel::Value, std::string> logLevels = {
        {LogLevel::Error,   "ERROR"},
        {LogLevel::Warning, "WARNING"},
        {LogLevel::Debug,   "DEBUG"},
        {LogLevel::Info,    "INFO"},
        {LogLevel::Verbose, "VERBOSE"},
    };


    template<typename A, typename B>
    std::pair<B, A> flipPair(const std::pair<A, B>& pair)
    {
        return std::pair<B,A>(pair.second, pair.first);
    }

    template<typename A, typename B>
    std::unordered_map<B, A> flipMap(const std::unordered_map<A, B>& map)
    {
        std::unordered_map<B, A> flippedMap;
        std::transform(map.begin(), map.end(), std::inserter(flippedMap, flippedMap.begin()), flipPair<A, B>);
        return flippedMap;
    }
}

LogLevel::LogLevel(std::string name)
{
    std::unordered_map<std::string, LogLevel::Value> flippedLogLevels = flipMap(logLevels);
    std::transform(name.begin(), name.end(), name.begin(), toupper);
    value = flippedLogLevels.at(name);
}


LogLevel::operator std::string() const noexcept
{
    return logLevels.at(value);
}


std::ostream& operator<<(std::ostream& os, const LogLevel& level) noexcept
{
    os << static_cast<std::string>(level);
    return os;
}


#include "Logger.h"

namespace PM
{
    MultiLogger Logger = MultiLogger({LogStream(LogLevel::Error, LogLevel::Verbose, std::cout, true)});
}
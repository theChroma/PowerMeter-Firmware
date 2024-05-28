#include "Logger.h"

MultiLogger Logger = MultiLogger({LogStream(LogLevel::Error, LogLevel::Verbose, &std::cout, true)});
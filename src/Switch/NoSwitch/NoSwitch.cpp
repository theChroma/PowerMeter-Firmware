#include "NoSwitch.h"
#include "Logger/Logger.h"


NoSwitch::NoSwitch(const json &configJson) noexcept
{
    Logger[LogLevel::Info] << "Configured no switch sucessfully." << std::endl;
}


tl::optional<bool> NoSwitch::getState() const noexcept
{
    return tl::nullopt;
}

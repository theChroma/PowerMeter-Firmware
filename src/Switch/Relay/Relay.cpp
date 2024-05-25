#ifdef ESP32

#include "Relay.h"
#include "JsonResource/BasicJsonResource/BasicJsonResource.h"
#include "Filesystem/File/LittleFsFile/LittleFsFile.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include <Arduino.h>


namespace
{
    BasicJsonResource stateResource(
        std::unique_ptr<Filesystem::File>(
            new Filesystem::LittleFsFile("/Relay/State.json")
        )
    );
}


Relay::Relay(const json &configJson)
{
    try
    {
        m_pin = configJson.at("pin");
        m_isNormallyOpen = configJson.at("isNormallyOpen");
        bool state = stateResource.deserializeOr(false);
        pinMode(m_pin, OUTPUT);
        digitalWrite(m_pin,  m_isNormallyOpen ? state : !state);
        Logger[LogLevel::Info] << "Relay configured sucessfully." << std::endl;
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to configure Relay");
        throw;
    }
}


tl::optional<bool> Relay::getState() const noexcept
{
    bool state = digitalRead(m_pin);
    return m_isNormallyOpen ? state : !state;
}


void Relay::setState(bool state)
{
    try
    {
        stateResource.serialize(state);
        digitalWrite(m_pin, m_isNormallyOpen ? state : !state);
    }
    catch (...)
    {
        ExceptionTrace::trace(SOURCE_LOCATION + "Failed to set Relay state");
        throw;
    }
}

#endif
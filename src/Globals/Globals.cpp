#ifdef ESP32

#include "Globals.h"
#include "Encryption/Certificate.h"
#include "Encryption/PrivateKey.h"


namespace Globals
{
    const std::string FirmwareVersion = "0.0.0";
    const JsonURI LoggerConfigURI = JsonURI("/Config/Logger.json");
    const JsonURI WiFiConfigURI = JsonURI("/Config/Wifi.json");
    const JsonURI MeasuringConfigURI = JsonURI("/Config/Measuring.json");
    const JsonURI TrackersConfigURI = JsonURI("/Config/Trackers.json");
    const JsonURI RelayConfigURI = JsonURI("/Config/Relay.json");
    const JsonURI RelayStateURI = JsonURI("/Relay/State.json");

    ACPowerMeter PowerMeter;
    DS3231 RTC;
    std::unordered_map<std::string, Tracker> Trackers;
    HTTPSServer Server = HTTPSServer(
        SSLContext("/SSL/Certificate.der"),
        SSLContext("/SSL/PrivateKey.der"),
        443, 
        {{"Access-Control-Allow-Origin", "*"}}
    );
    RestAPI API = RestAPI(Server, "/api");
}

#endif
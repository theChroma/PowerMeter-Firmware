#ifdef ESP32

#include "WifiScan.h"
#include "SourceLocation/SourceLocation.h"
#include <WiFi.h>

namespace
{
    std::string authenticationToString(wifi_auth_mode_t authentication)
    {
        switch (authentication)
        {
            case WIFI_AUTH_OPEN:
                return "Open";
            case WIFI_AUTH_WEP:
                return "WEP";
            case WIFI_AUTH_WPA_PSK:
                return "WPA";
            case WIFI_AUTH_WPA2_PSK:
                return "WPA2";
            case WIFI_AUTH_WPA_WPA2_PSK:
                return "WPA+WPA2";
            case WIFI_AUTH_WPA2_ENTERPRISE:
                return "WPA2 Enterprise";
            case WIFI_AUTH_WPA3_PSK:
                return "WPA3";
            case WIFI_AUTH_WPA2_WPA3_PSK:
                return "WPA2+WPA3";
            case WIFI_AUTH_WAPI_PSK:
                return "WAPI";
        }
        return "Unknown";
    }
}


WifiScan::WifiScan(uint32_t timeout_ms)
{
    WiFi.scanNetworks(true, false, true);
    uint32_t startTime = millis();
    while (true)
    {
        m_scannedNetworkCount = WiFi.scanComplete();
        if (m_scannedNetworkCount >= 0)
            break;
        if (m_scannedNetworkCount == WIFI_SCAN_FAILED)
            throw std::runtime_error(SOURCE_LOCATION + "Failed to scan WiFi networks");
        if (millis() - startTime > timeout_ms)
            throw std::runtime_error(SOURCE_LOCATION + "Scanning WiFi networks timed out");
        delay(100);
    }
}


WifiScan::~WifiScan()
{
    WiFi.scanDelete();
}


json WifiScan::toJson()
{
    json networksJson = json::array_t();
    for (int8_t i = 0; i < m_scannedNetworkCount; i++)
    {
        networksJson.push_back({
            {"ssid", WiFi.SSID(i).c_str()},
            {"rssi", WiFi.RSSI(i)},
            {"bssid", WiFi.BSSIDstr(i).c_str()},
            {"authentication", authenticationToString(WiFi.encryptionType(i))},
            {"channel", WiFi.channel(i)},
        });
    }
    return networksJson;
}

#endif
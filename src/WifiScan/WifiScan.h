#pragma once

#include <json.hpp>

class WifiScan
{
public:
    WifiScan(uint32_t timeout_ms = 10000);
    ~WifiScan();

    json toJson();

private:
    int8_t m_scannedNetworkCount = 0;
};
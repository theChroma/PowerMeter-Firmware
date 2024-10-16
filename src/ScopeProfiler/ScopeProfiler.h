#pragma once

#include <string>

class ScopeProfiler
{
public:
    ScopeProfiler(const std::string& name);
    ~ScopeProfiler();

private:
    std::string m_name;
    uint32_t m_profileStartTime_us;
};
#pragma once

#include <string>
#include <ESPAsyncWebServer.h>

namespace FileBrowser
{
    void serve(AsyncWebServer& server, const std::string& uri);
}
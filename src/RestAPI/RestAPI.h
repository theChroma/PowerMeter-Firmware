#pragma once

#include "Version/Version.h"
#include <json.hpp>
#include <functional>
#include <string>
#include <ESPAsyncWebServer.h>

namespace PM
{
    namespace Http
    {
        using Header = std::pair<std::string, std::string>;
        using HeaderMap = std::unordered_map<std::string, std::string>;
    }

    class RestAPI
    {
    public:
        struct JsonResponse
        {
            JsonResponse(
                const json& data = nullptr,
                uint16_t statusCode = 200,
                const Http::HeaderMap& headers = {}
            ) :
                data(data),
                statusCode(statusCode),
                headers(headers)
            {}
            json data;
            uint16_t statusCode;
            Http::HeaderMap headers;
        };

        using JsonHandler = std::function<JsonResponse(const json&, const Version&)>;

        RestAPI(AsyncWebServer& server, const Version& apiVersion, const std::string& baseUri = "") noexcept;
        void handle(const std::string& uri, WebRequestMethod method, const JsonHandler& handler) noexcept;

    private:
        AsyncWebServer& m_server;
        Version m_apiVersion;
        std::string m_baseUri;
    };
}


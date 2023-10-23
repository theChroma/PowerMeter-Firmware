#ifndef RESTAPI_H
#define RESTAPI_H

#include "HTTPServer/HTTPServer.h"

#include <json.hpp>
#include <functional>
#include <string>

class RestAPI
{
public:
    struct JsonResponse
    {
        JsonResponse(
            const json& data = nullptr,
            HTTP::StatusCode status = HTTP::StatusCode::OK,
            const HTTP::HeaderMap& headers = {}
        ) :
            data(data),
            status(status),
            headers(headers)
        {}
        json data;
        HTTP::StatusCode status;
        HTTP::HeaderMap headers;
    };

    using JsonHandler = std::function<JsonResponse(const json&)>;
    using HeaderList = std::vector<HTTP::Header>;

    RestAPI(
        HTTPServer& server, 
        const std::string& baseURI = ""
    ) noexcept;

    void registerURI(const std::string& uri, HTTP::Method method, const JsonHandler& handlerCallback) noexcept;

private:
    HTTPServer& m_server;
    std::string m_baseURI;
};

#endif

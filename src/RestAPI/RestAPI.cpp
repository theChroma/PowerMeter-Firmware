#ifdef ESP32

#include "RestApi.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include <sstream>


namespace
{
    std::string escapeSlashes(const std::string& input)
    {
        std::string result;
        for (char c : input)
        {
            if (c == '/')
                result += '\\';
            result += c;
        }
        return result;
    }

    std::string methodToString(WebRequestMethod method)
    {
        switch (method)
        {
            case HTTP_GET:
                return "GET";
            case HTTP_POST:
                return "POST";
            case HTTP_DELETE:
                return "DELETE";
            case HTTP_PUT:
                return "PUT";
            case HTTP_PATCH:
                return "PATCH";
            case HTTP_HEAD:
                return "HEAD";
            case HTTP_OPTIONS:
                return "OPTIONS";
        }
        return "";
    }
}

RestApi::RestApi(AsyncWebServer &server, const Version& apiVersion, const std::string &baseUri) noexcept :
    m_server(server),
    m_apiVersion(apiVersion),
    m_baseUri(baseUri)
{
    std::stringstream uriRegex;
    uriRegex << '^' << escapeSlashes(m_baseUri) << "/(.*)$";
    m_server.on(uriRegex.str().c_str(), HTTP_OPTIONS, [](AsyncWebServerRequest* request){
        AsyncWebServerResponse* response = request->beginResponse(200);
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "*");
        request->send(response);
    });
}

void RestApi::handle(const std::string &uri, WebRequestMethod method, const JsonHandler &handler) noexcept
{
    static bool alreadyHandled = false;
    alreadyHandled = false;
    auto serverHandler = [this, handler, method, uri](AsyncWebServerRequest* request, uint8_t* data, size_t length, size_t index, size_t total) {
        JsonResponse jsonResponse(json(), 500);
        try
        {
            Version requestedApiVersion(request->pathArg(0).c_str());
            if (requestedApiVersion > m_apiVersion)
            {
                std::stringstream errorMessage;
                errorMessage
                    << "The requested API version (v" << requestedApiVersion << ") is not available. "
                    << "The latest available API version is v" << m_apiVersion << ". "
                    << "Try updating to the latest firmware.";
                throw std::runtime_error(SOURCE_LOCATION + errorMessage.str());
            }
            json requestJson;
            if (length > 0)
            {
                std::string body(reinterpret_cast<const char*>(data));
                body.resize(length);
                requestJson = json::parse(body);
            }
            try
            {
                jsonResponse = handler(JsonRequest {
                    .data = requestJson,
                    .version = requestedApiVersion,
                    .serverRequest = *request,
                });
            }
            catch (...)
            {
                ExceptionTrace::trace(SOURCE_LOCATION + "Request handler for \"" + methodToString(method) + " " + uri + "\" failed");
                throw;
            }
        }
        catch (...)
        {
            Logger[LogLevel::Error]
                << "Exception occurred at "
                << SOURCE_LOCATION << "\r\n"
                << ExceptionTrace::what(false) << std::endl;
            jsonResponse.data = ExceptionTrace::get();
        }
        AsyncWebServerResponse* response = request->beginResponse(
            jsonResponse.statusCode,
            "application/json",
            (jsonResponse.data == nullptr && jsonResponse.statusCode == 204 ) ? "" : jsonResponse.data.dump(1, '\t').c_str()
        );
        response->addHeader("Access-Control-Allow-Origin", "*");
        for (const auto& header : jsonResponse.headers)
            response->addHeader(header.first.c_str(), header.second.c_str());
        request->send(response);
        delay(100);
        jsonResponse.doAfterSend();
    };

    std::stringstream uriRegex;
    uriRegex << '^' << escapeSlashes(m_baseUri) << "/v(.*)" << escapeSlashes(uri) << "$";

    m_server.on(
        uriRegex.str().c_str(),
        method,
        [serverHandler](AsyncWebServerRequest* request){
            if (!alreadyHandled)
                serverHandler(request, nullptr, 0, 0, 0);
            alreadyHandled = false;
        },
        nullptr,
        [serverHandler](AsyncWebServerRequest* request, uint8_t* data, size_t length, size_t index, size_t total) {
            alreadyHandled = true;
            serverHandler(request, data, length, index, total);
        }
    );
}

#endif
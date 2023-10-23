#ifndef HTTPSSERVER_H
#define HTTPSSERVER_H

#include "HTTPServer/HTTPServer.h"
#include "SSLContext/SSLContext.h"


class HTTPSServer : public HTTPServer
{
public:
    HTTPSServer(
        const SSLContext& certificate,
        const SSLContext& privateKey,
        uint16_t port = 443,
        const HTTP::HeaderMap& defaultHeaders = {}
    );
    void start() override;
    void stop() override;

private:
    SSLContext m_certificate;
    SSLContext m_privateKey;
};

#endif
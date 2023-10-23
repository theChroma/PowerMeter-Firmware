#ifdef ESP32

#include "HTTPSServer.h"
#include "Encryption/Certificate.h"
#include "Encryption/PrivateKey.h"
#include "Log/Log.h"
#include <esp_https_server.h>


HTTPSServer::HTTPSServer(
    const SSLContext& certificate,
    const SSLContext& privateKey,
    uint16_t port,
    const HTTP::HeaderMap& defaultHeaders
) : 
    m_certificate(certificate),
    m_privateKey(privateKey)
{
    m_port = port;
    m_defaultHeaders = defaultHeaders;
}


void HTTPSServer::start()
{
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.httpd.stack_size = 8192;
    config.httpd.max_uri_handlers = 256;
    config.httpd.max_resp_headers = 32;
    config.httpd.core_id = 0;
    config.port_secure = m_port;
    config.cacert_pem = m_certificate.getContext();
    config.cacert_len = m_certificate.getLength();
    config.prvtkey_pem = m_privateKey.getContext();
    config.prvtkey_len = m_privateKey.getLength();
    httpd_ssl_start(&m_server, &config);

    Logger[Level::Debug] << "Certificate Length: " << config.cacert_len << std::endl;
    // for(size_t i = 0; i < config.cacert_len; i++)
    // {
    //     printf("0x%02X\n", config.cacert_pem[i]);
    // }
    Logger[Level::Debug] << "Private Key Length: " << config.prvtkey_len << std::endl;
    // for(size_t i = 0; i < config.prvtkey_len; i++)
    // {
    //     printf("0x%02X\n", config.prvtkey_pem[i]);
    // }
}


void HTTPSServer::stop()
{
    httpd_ssl_stop(m_server);
}

#endif
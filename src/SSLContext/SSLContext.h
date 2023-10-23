#ifndef SSLCONTEXT_H
#define SSLCONTEXT_H

#include <vector>
#include <string>

class SSLContext
{
public:
    SSLContext(const std::string& filePath);

    ~SSLContext();

    const uint8_t* getContext();
    size_t getLength() const noexcept;

private:
    std::string m_filePath;
    std::vector<uint8_t> m_context;
};

#endif
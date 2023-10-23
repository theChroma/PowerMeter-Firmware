#include "SSLContext.h"
#include "Log/Log.h"
#include <fstream>

SSLContext::SSLContext(const std::string& filePath) : m_filePath(filePath)
{
    Logger[Level::Debug] << "Constructing SSLContext " << m_filePath << std::endl;
}

SSLContext::~SSLContext()
{
    Logger[Level::Debug] << "Destructing SSLContext " << m_filePath << std::endl;
}

const uint8_t* SSLContext::getContext()
{
    std::ifstream file(m_filePath);
    if(!file.good())
        throw std::runtime_error('"' + m_filePath + "\" is not a valid filepath");

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    m_context.resize(fileSize);
    file.read(reinterpret_cast<char*>(m_context.data()), fileSize);
    return m_context.data();
}


size_t SSLContext::getLength() const noexcept
{
    return m_context.size();
}
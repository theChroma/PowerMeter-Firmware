#include "LogStream.h"
#include <algorithm>
#include <exception>
#include <fstream>


using PM::LogStream;

LogStream::LogStream(LogLevel minLevel, LogLevel maxLevel, std::ostream& stream, bool showLevel) noexcept :
    m_minLevel(minLevel),
    m_maxLevel(maxLevel),
    m_stream(stream),
    m_showLevel(showLevel)
{}


std::ostream& LogStream::operator[](LogLevel level) noexcept
{
    if(level >= m_minLevel && level <= m_maxLevel)
    {
        if(m_showLevel)
            m_stream << "[" << level << "] ";
        return m_stream;
    }

    class NullStreamBuffer : public std::streambuf
    {
    public:
        int overflow(int c) override { return c; }
    };

    static NullStreamBuffer nullBuffer;
    static std::ostream nullStream(&nullBuffer);
    return nullStream;
}

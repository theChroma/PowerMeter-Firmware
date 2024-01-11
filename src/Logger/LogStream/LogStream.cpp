#include "LogStream.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include "SourceLocation/SourceLocation.h"
#include <algorithm>
#include <fstream>

using namespace PM;

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

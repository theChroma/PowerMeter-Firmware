#include "MultiLogger.h"

using PM::MultiLogger;


MultiLogger::MultiLogger(const std::vector<LogStream> &streams) : m_streams(streams)
{}

std::ostream &MultiLogger::operator[](LogLevel level) noexcept
{
    struct MultiStreamBuffer : public std::streambuf
    {
        virtual int overflow(int c) override
        {
            for (auto& buffer : buffers)
                buffer->sputc(c);
            return c;
        }

        std::vector<std::streambuf*> buffers;
    };

    static MultiStreamBuffer multiStreamBuffer;
    multiStreamBuffer.buffers.clear();
    for (auto& stream : m_streams)
        multiStreamBuffer.buffers.push_back(stream[level].rdbuf());
    static std::ostream multiStream(&multiStreamBuffer);
    return multiStream;
}
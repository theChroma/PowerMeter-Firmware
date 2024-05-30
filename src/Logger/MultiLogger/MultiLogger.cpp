#include "MultiLogger.h"
#include <mutex>


MultiLogger::MultiLogger(const std::vector<LogStream> &streams) : m_streams(streams)
{}


std::ostream &MultiLogger::operator[](LogLevel level) noexcept
{
    static std::mutex syncMutex;
    struct MultiStreamBuffer : public std::streambuf
    {
        virtual int overflow(int c) override
        {
            for (auto& buffer : buffers)
                buffer->sputc(c);
            return c;
        }

        virtual int sync() override
        {
            std::lock_guard<std::mutex> lock(syncMutex);
            for (auto& buffer : buffers)
                buffer->pubsync();
            return 0;
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
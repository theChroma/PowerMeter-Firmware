#include "ExceptionTrace.h"
#include <sstream>
#include <algorithm>

namespace
{
    ExceptionTrace::Traces traces;

    void traceCurrent(ExceptionTrace::Traces& traces)
    {
        try
        {
            std::exception_ptr currentException = std::current_exception();
            if(currentException)
                std::rethrow_exception(currentException);
        }
        catch(const std::exception& exeception)
        {
            traces.push_front(exeception.what());
        }
        catch(const char* exeception)
        {
            traces.push_front(exeception);
        }
        catch(...)
        {
            traces.push_front("Unexpected Exception");
        }
    }
}


void ExceptionTrace::trace(const std::string& message) noexcept
{
    traces.push_back(message);
}


void ExceptionTrace::clear() noexcept
{
    traces.clear();
}


ExceptionTrace::Traces ExceptionTrace::get(bool clearTraces) noexcept
{
    Traces tracesCopy = traces;
    traceCurrent(tracesCopy);
    if (clearTraces)
        ExceptionTrace::clear();
    std::reverse(tracesCopy.begin(), tracesCopy.end());
    return tracesCopy;
}


std::string ExceptionTrace::what(bool clearTraces, size_t indentLevel, char indentChar) noexcept
{
    Traces tracesCopy = traces;
    traceCurrent(tracesCopy);
    std::stringstream what;
    for(size_t i = 0; i < tracesCopy.size(); i++)
        what << std::string(indentLevel * i, indentChar) << tracesCopy.at(tracesCopy.size() - i - 1) << "\r\n";
    if (clearTraces)
        ExceptionTrace::clear();
    return what.str();
}

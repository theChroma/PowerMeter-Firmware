#include "Entry.h"
#include "Logger/Logger.h"

json Filesystem::Entry::toJson() const
{
    return {
        {"name", getName()},
        {"path", getPath()},
    };
}
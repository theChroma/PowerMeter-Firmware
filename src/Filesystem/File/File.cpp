#include "File.h"

json Filesystem::File::toJson() const
{
    json entryJson = Entry::toJson();
    entryJson["type"] = "File";
    entryJson["lastWriteTimestamp"] = getLastWriteTimestamp();
    return entryJson;
}
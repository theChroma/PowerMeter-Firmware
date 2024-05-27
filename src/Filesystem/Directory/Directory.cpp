#include "Directory.h"

using namespace Filesystem;

json Directory::toJson() const
{
    json entryJson = Entry::toJson();

    json::array_t childrenJson;
    for (const auto& entry : getEntries())
        childrenJson.push_back(entry->toJson());

    entryJson["type"] = "Directory";
    entryJson["children"] = childrenJson;
    return entryJson;
}
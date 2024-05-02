#ifdef ESP32

#include "FileBrowser.h"
#include "Logger/Logger.h"
#include "SourceLocation/SourceLocation.h"
#include <dirent.h>
#include <stdexcept>
#include <sstream>
#include <LittleFS.h>


namespace
{
    std::string getFilesystemTreeHtml(const std::string& uri, const std::string& path = "")
    {
        DIR* directory = opendir(path.c_str());
        std::ostringstream html;
        if (!directory)
        {
            Logger[LogLevel::Error] << SOURCE_LOCATION << "Failed to generate Filesystem Tree for" << path << std::endl;
            html.str("");
            html << "<i style=\"color: red;\">Failed to generate Filesystem Tree for" << path << "</i>";
            return html.str();
        }
        html << "<ul>";
        struct dirent* directoryEntry;
        while (directoryEntry = readdir(directory))
        {
            html << "<li>";
            if (directoryEntry->d_type == DT_DIR)
            {
                html << directoryEntry->d_name;
                html << getFilesystemTreeHtml(uri, path + "/" + directoryEntry->d_name);
            }
            else
            {
                html << "<a href=\"" << uri << path << "/" << directoryEntry->d_name << "\">" << directoryEntry->d_name << "</a>";
            }
            html << "</li>";
        }
        closedir(directory);
        html << "</ul>";
        return html.str();
    }
}



void FileBrowser::serve(AsyncWebServer& server, const std::string& uri)
{
    server.serveStatic(uri.c_str(), LittleFS, "/")
        .setDefaultFile("/FileBrowser/index.html")
        .setTemplateProcessor([uri](String){
            return String(getFilesystemTreeHtml(uri).c_str());
        });
}

#endif
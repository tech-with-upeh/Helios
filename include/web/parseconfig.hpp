#pragma once

#include <string>
#include <vector>

struct FileEntry {
    std::string path;  // optional, empty for default
    std::string file;
};

struct WebLinks {
    std::vector<FileEntry> scripts;
    std::vector<FileEntry> css;
    std::string favicon;
};
WebLinks parseHeliosWebConfig(const std::string& path);

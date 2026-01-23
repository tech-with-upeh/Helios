#pragma once

#include <string>
#include <vector>

struct WebLinks {
    std::vector<std::string> scripts;
    std::vector<std::string> css;
    std::string favicon;
};

WebLinks parseHeliosWebConfig(const std::string& path);

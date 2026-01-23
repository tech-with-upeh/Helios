#include "parseconfig.hpp"

#include <fstream>
#include <stdexcept>

static inline std::string trim(const std::string& s) {
    const char* ws = " \t\n\r";
    auto start = s.find_first_not_of(ws);
    auto end = s.find_last_not_of(ws);
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

WebLinks parseHeliosWebConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Failed to open Web config file, run command from project Root");

    WebLinks links;

    std::string line;
    bool inWeb = false;
    std::string currentKey;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "[web]") {
            inWeb = true;
            continue;
        }

        if (line == "[end]") {
            break;
        }

        if (!inWeb) continue;

        // Detect keys
        if (line == "script:") {
            currentKey = "script";
            continue;
        }

        if (line == "css:") {
            currentKey = "css";
            continue;
        }

        if (line == "favicon:") {
            currentKey = "favicon";
            continue;
        }

        // URL line
        if (line.starts_with("http")) {
            if (currentKey == "script")
                links.scripts.push_back(line);
            else if (currentKey == "css")
                links.css.push_back(line);
        } else if (currentKey == "favicon") {
            links.favicon = line;
        } else {
            if (currentKey == "script")
                links.scripts.push_back(line);
            else if (currentKey == "css")
                links.css.push_back(line);
        }
    }

    return links;
}

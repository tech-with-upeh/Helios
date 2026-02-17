
#include "json.hpp"
#include "parseconfig.hpp"

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

WebLinks parseHeliosWebConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Failed to open Web config file, run command from project Root");

    json j;
    file >> j;

    WebLinks links;

    // --- Scripts ---
    for (const auto& s : j["web"]["scripts"]) {
        if (s.is_string()) {
            links.scripts.push_back({"", s.get<std::string>()}); // default
        } else if (s.is_object() && s.contains("file")) {
            std::string file = s["file"].get<std::string>();
            std::string path = s.value("path", ""); // optional
            links.scripts.push_back({path, file});
        }
    }

    // --- CSS ---
    for (const auto& c : j["web"]["css"]) {
        if (c.is_string()) {
            links.css.push_back({"", c.get<std::string>()});
        } else if (c.is_object() && c.contains("file")) {
            std::string file = c["file"].get<std::string>();
            std::string path = c.value("path", "");
            links.css.push_back({path, file});
        }
    }

    // --- Favicon ---
    links.favicon = j["web"].value("favicon", "");

    return links;
}

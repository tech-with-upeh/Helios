#include <uwebsockets/App.h>
#include "heliosfilewatcher.hpp"
static const std::string WEB_ROOT = "./web";
static const std::string PUBLIC_ROOT = "./public";

/* ================= MIME ================= */
std::string mime_type(std::string_view path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".js")) return "application/javascript";
    if (path.ends_with(".css")) return "text/css";
    if (path.ends_with(".png")) return "image/png";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
    if (path.ends_with(".svg")) return "image/svg+xml";
    if (path.ends_with(".wasm")) return "application/wasm";
    if (path.ends_with(".ico")) return "image/x-icon";
    if (path.ends_with(".gif")) return "image/gif";
    return "application/octet-stream";
}

bool read_file(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    out.assign(std::istreambuf_iterator<char>(file),
               std::istreambuf_iterator<char>());
    return true;
}

/* ================= MAIN SERVER ================= */
int main() {
    
}

#include "httpserver.hpp"
#include <ranges>

const std::string WEB_ROOT = "./web";

/* ======================= mime types ======================= */

std::string mime_type(const std::string& path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".js"))   return "application/javascript";
    if (path.ends_with(".css"))  return "text/css";
    if (path.ends_with(".png"))  return "image/png";
    if (path.ends_with(".jpg"))  return "image/jpeg";
    if (path.ends_with(".svg"))  return "image/svg+xml";
    if (path.ends_with(".wasm")) return "application/wasm";
    if (path.ends_with(".ico"))  return "image/x-icon";
    if (path.ends_with(".json")) return "application/json";
    return "application/octet-stream";
}


/* ======================= helpers ======================= */

bool read_file(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    out.assign(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    return true;
}

/* ======================= http_session ======================= */

http_session::http_session(tcp::socket socket)
    : socket_(std::move(socket)), timer_(socket_.get_executor()) {}


void http_session::run() {
    start_timer();
    http::async_read(socket_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
         
            self->handle();
            self->timer_.cancel();  // Cancel with error code to not throw
            if (ec) {
                std::cerr << "Read error: " << ec.message() << "\n";
                return;  // Let session die naturally
            } 
        });
}

void http_session::handle() {
    // ---- WebSocket upgrade ----
    if (websocket::is_upgrade(req_)) {
        std::make_shared<websocket_session>(
            std::move(socket_))->run(std::move(req_));
        return;
    }

    // ---- Allowed methods ----
    auto method = req_.method();
    if (method != http::verb::get &&
        method != http::verb::head &&
        method != http::verb::options)
    {
        send_method_not_allowed();
        return;
    }

    // ---- OPTIONS (CORS / Dev) ----
    if (method == http::verb::options) {
        http::response<http::empty_body> res{
            http::status::ok, req_.version()
        };
        res.set(http::field::allow, "GET, HEAD, OPTIONS");
        res.prepare_payload();
        http::async_write(socket_, res,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
        }
            });
        return;
    }

    // ---- GET or HEAD ----
    std::string target = std::string(req_.target());
    if (target.empty())
        target = "/";

    if (target.find("..") != std::string::npos &&
    !(target.rfind("../public/", 0) == 0 ||
      target.rfind("..\\public\\", 0) == 0)) {
        send_not_found();
        return;
    }


    // normalize
    if (target == "/")
        target = "/index.html";
    std::string full_path = WEB_ROOT + target;

    if (target.find(".png") != std::string::npos 
        || target.find(".jpg") != std::string::npos
        || target.find(".svg") != std::string::npos 
        || target.find(".ico") != std::string::npos 
        || target.find(".jpeg") != std::string::npos
        || target.find(".gif") != std::string::npos
    ) {
        full_path =  "./public" + target;
    }
    
    std::string body;
    std::cout << http::to_string(req_.method()) << " -> " << target << std::endl;

    bool file_exists = read_file(full_path, body);
    if (!file_exists) {
        full_path = WEB_ROOT + "/index.html";
        if (!read_file(full_path, body)) {
            send_not_found();
            return;
        }
    }

    http::response<http::string_body> res{
        http::status::ok, req_.version()
    };
    res.set(http::field::content_type, mime_type(full_path));

    // HEAD request → empty body
    if (method == http::verb::head) {
        res.set("Helios-Dev-Server", "Helios DevServer v1.0");
        res.set("Helios-Node", "localhost:8000");
        res.body() = "";
    } else {
        res.body() = std::move(body);
    }

    res.prepare_payload();
    http::async_write(socket_, res,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
        }
        });
}

void http_session::send_method_not_allowed() {
    http::response<http::string_body> res{
        http::status::method_not_allowed, req_.version()
    };
    res.set(http::field::allow, "GET, HEAD, OPTIONS");
    res.body() = "405 Method Not Allowed";
    res.prepare_payload();
    http::async_write(socket_, res,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
        }
        });
}

void http_session::send_not_found() {
    http::response<http::string_body> res{
        http::status::not_found, req_.version()
    };
    res.set(http::field::content_type, "text/plain");
    res.body() = "404 Not Found";
    res.prepare_payload();

    http::async_write(socket_, res,
        [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
        }
        });
}

void http_session::start_timer() {
    timer_.expires_after(std::chrono::seconds(10)); // 10s timeout
    timer_.async_wait([self = shared_from_this()](const boost::system::error_code& ec){
        if (ec == net::error::operation_aborted) {
            std::cout << "timer cancelled, closing connection\n";
            return; // Timer was cancelled, all good
        }
        if (!ec) {
            std::cout << "Client timed out, closing connection\n";
            boost::system::error_code close_ec;
            self->socket_.close(close_ec); // Use error_code version to not throw
        }
    });
}

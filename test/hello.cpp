#include <uWebSockets/App.h>
#include <iostream>

int main() {
    // uWS::App() is for HTTP, uWS::SSLApp() is for HTTPS
    uWS::App().get("/", [](auto *res, auto *req) {
        
        // You must end the response or the browser will hang
        res->end("Hello World from uWebSockets!");
        
    }).listen(3000, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Server started on port 3000" << std::endl;
        } else {
            std::cerr << "Failed to load port 3000" << std::endl;
        }
    }).run();

    return 0;
}

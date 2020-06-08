# XyzCpp
An implementation of the Xyz-protocol in C++.
This library is header-only (though linking with appropriate shared libraries is required).

### Examples

#### Server Example
A simple echo server.
```C++
#include <iostream>
#include <vector>
#include <XyzCpp/XyzServer.hpp>
#include <XyzCpp/XyzClient.hpp>
#include <XyzCpp/XyzUtils.hpp>
int main () {
    XyzCpp::XyzServer server;
    server.onConnect = [] (XyzCpp::XyzClient& client) {
        std::cout << "[server] Client connected to us\n";

        client.onDisconnect = [] (XyzUtils::Error error) {
            std::cout << "[server] Client disconnected: " << error.toString() << "\n";
        };

        client.onMessage = [&client] (std::vector<std::byte> data, int type) {
            std::cout << "[server received] " << type << ": ";
            for (std::byte b : data) {
                std::cout << static_cast<char>(b);
            }
            std::cout << "\n";

            client.send(data, type);
        };

        client.read();
    };

    // Sit here, since the server is running on another thead
    char c;
    while (true) {
        std::cin.get(c);
    }
}
```

#### Client Example
```C++
#include <iostream>
#include <vector>
#include <XyzCpp/XyzClient.hpp>
#include <XyzCpp/XyzUtils.hpp>
int main () {
    XyzCpp::XyzClient client;

    client.onConnect = [] () {
        std::cout << "[client] Client connected to server\n";
    };
    client.onDisconnect = [] (XyzUtils::Error error) {
        std::cout << "[client] Disconnected: " << error.toString() << "\n";
    };
    client.onMessage = [] (std::vector<std::byte> data, int type) {
        std::cout << "[client received] " << type << ": ";
        for (std::byte b : data) {
            std::cout << static_cast<char>(b);
        }
        std::cout << "\n";
    };

    if (!client.connect("127.0.0.1", 1234)) {
        // Disconnect handler will be called with error
        std::cout << "Could not connect.";
        return 1;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        client.send(line, 1);
    }
}
```

### Dependencies

- boost.asio (networking)
- boost.iostreams (deflation/inflation with zlib module)
- zlib
- openssl (for sha256, and md5, pkbdf2, aes, ecdh)
(Also uses the libraries they depend upon on specific platforms, see meson.build for list).
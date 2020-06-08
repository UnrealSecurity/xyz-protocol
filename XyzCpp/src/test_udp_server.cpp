#include <include/XyzUdpClient.hpp>
#include <iostream>

// Simple echo server
int main () {
	XyzCpp::XyzUdpClient server;
	server.onMessage = [&server] (XyzCpp::XyzUdpClient::Endpoint endpoint, std::vector<std::byte> data, int type) {
		std::cout << "Received message\n";
		server.send(endpoint, data, type);
	};

	server.bind("127.0.0.1", 1235);

	char line[2048];
	while (std::cin.getline(line, 2048)) {}
}
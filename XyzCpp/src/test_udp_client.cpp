#include <include/XyzUdpClient.hpp>
#include <iostream>

// Simple echo server
int main () {
	XyzCpp::XyzUdpClient client;
	client.onMessage = [&client] (XyzCpp::XyzUdpClient::Endpoint endpoint, std::vector<std::byte> data, int type) {
		std::cout << "Received message from server: " << std::hex;
		for (std::byte v : data) {
			std::cout << static_cast<char>(v);
		}
		std::cout << std::dec << "\n";
	};

	XyzCpp::XyzUdpClient::Endpoint endpoint = XyzCpp::XyzUdpClient::getEndpoint("127.0.0.1", 1235);
	client.connect(endpoint);

	char line[256];
	while (std::cin.getline(line, 255)) {
		std::cout << "Sending: " << line << "\n";
		client.send(reinterpret_cast<std::byte*>(line), strlen(line), 1);
	}
}
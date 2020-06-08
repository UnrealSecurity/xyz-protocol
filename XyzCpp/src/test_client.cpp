#include <iostream>

#include <include/XyzClient.hpp>
#include <include/XyzKeyExchange.hpp>

#include <stdio.h>
#include <readline/readline.h>

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>


using boost::asio::ip::tcp;

// minor utility function to output bytes to an ostream.
static void output (std::ostream& output, const std::vector<std::byte>& data, bool hex=true) {
	if (hex) {
		output << std::hex;
	}
	for (std::byte value : data) {
		output << static_cast<int>(value) << " ";
	}
	if (hex) {
		output << std::dec;
	}
}

static void outputString (std::ostream& output, const std::vector<std::byte>& data) {
	for (std::byte value : data) {
		output << static_cast<char>(value);
	}
}

static std::vector<std::byte> fromString (std::string str) {
	std::vector<std::byte> result;
	for (char c : str) {
		result.push_back(std::byte(static_cast<unsigned char>(c)));
	}
	return result;
}

int main (int argc, char** argv) {
	using namespace XyzCpp;

	std::string address;
	if (argc == 1) {
		std::cout << "No address defined. Defaulting to localhost\n";
		address = "localhost";
	} else {
		address = argv[1];
	}

	XyzClient client;

	XyzKeyExchange key_exchange;
	std::vector<std::byte> shared_key;

	bool sent_test_echo = false;

	client.onConnect = [&key_exchange, &client] () {
		std::cout << "[client] Client connected to server\n";
		client.send(key_exchange.getLocalPublicKey(), 2); // public key exchange
	};
	client.onDisconnect = [] (XyzUtils::Error error) {
		std::cout << "[client] Disconnected: " << error.toString() << "\n";
	};
	client.onMessage = [&key_exchange, &client, &shared_key, &sent_test_echo] (std::vector<std::byte> data, int type) {
		std::cout << "[client received] " << type << ": ";
		output(std::cout, data);
		std::cout << "\n";
		std::cout << "Data size: " << data.size() << "\n";

		if (type == 2) { // key exchange
			std::cout << "Set remote public key\n";
			key_exchange.setRemotePublicKey(data);
			shared_key = key_exchange.getSharedSecretKey();

			std::cout << "Shared key: ";
			output(std::cout, shared_key);
			std::cout << "\n shared key size: " << shared_key.size() << "\n";

			if (!sent_test_echo) {
				sent_test_echo = true;
				std::string str = "The reality of god is me.";
				client.send(XyzUtils::encrypt(reinterpret_cast<std::byte*>(str.data()), str.size(), shared_key.data(), shared_key.size()), 3);
			}
		} else if (type == 3) {
			std::cout << "Received encrypted echo\n";
			auto unencrypted = XyzUtils::decrypt(data, shared_key);
			outputString(std::cout, unencrypted);
			std::cout << "\n";
		}
	};
	
	std::cout << "Connecting to: " << address << "\n";
	if (!client.connect(address, 1234)) {
		std::cout << "Failed to connect\n";
	}

	std::string line;
	while (std::getline(std::cin, line)) {
		if (shared_key.size() != 0) {
			client.send(XyzUtils::encrypt(reinterpret_cast<std::byte*>(line.data()), line.size(), shared_key.data(), shared_key.size()), 3);
		} else {
			std::cout << "Haven't exchange public keys\n";
		}
	}

	return 0;
}

#include <iostream>

#include <stdio.h>
#include <readline/readline.h>
#include <vector>

#include <include/XyzServer.hpp>
#include <include/XyzClient.hpp>
#include <include/XyzUtils.hpp>
#include <include/XyzKeyExchange.hpp>


struct Connection {
	XyzCpp::XyzClient& client;
	XyzCpp::XyzKeyExchange key_exchange;
	bool isSecure = false;
	std::vector<std::byte> secret;

	Connection (XyzCpp::XyzClient& t_client) : client(t_client) {}
};
enum class Type {
	Echo = 1,
	KeyExchange,
	SecureEcho,
};

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

int main () {
	XyzCpp::XyzServer server(1234);

	std::vector<Connection> connections;
	server.onConnect = [&server, &connections] (XyzCpp::XyzClient& client) {
		std::cout << "[Info] Received connection\n";
		connections.push_back(Connection{client});

		Connection& connection = connections.at(connections.size() - 1);
		client.onMessage = [&connection] (std::vector<std::byte> data, int itype) {
			if (connection.isSecure) {
				try {
					data = XyzCpp::XyzUtils::decrypt(data, connection.secret);
					std::cout << "[Info] Encrypted message decrptyed.\n";
				} catch (...) {
					std::cout << "[Error] Failed to decrypt message. Disconnecting...\n";
					connection.client.disconnect();
					return;
				}
			}

			if (itype != 1 && itype != 2 && itype != 3) {
				std::cout << "[Error] Type is unknown. Disconnecting...\n";
				connection.client.disconnect();
				return;
			}

			Type type = static_cast<Type>(itype);
			std::cout << "Type: " << itype << "\n";

			if (type == Type::Echo) {
				std::cout << "[Info] Echo\n";
				std::cout << "echo: ";
				outputString(std::cout, data);
				std::cout << "\n";
				connection.client.send(data, itype);
			} else if (type == Type::KeyExchange && !connection.isSecure) {
				std::cout << "[Info] Key exchange\n";
				try {
					std::cout << "Key from client: ";
					output(std::cout, data);
					std::cout << "\n";
					connection.key_exchange.setRemotePublicKey(data);
					connection.secret = connection.key_exchange.getSharedSecretKey();
					connection.isSecure = true;

					connection.client.send(connection.key_exchange.getLocalPublicKey(), static_cast<int>(Type::KeyExchange));
				} catch (std::exception& e) {
					std::cerr << "Exception: " << e.what() << "\n";
				}
			} else if (type == Type::SecureEcho && connection.isSecure) {
				std::cout << "[Info] Secure echoing data\n";
				std::cout << "secho: ";
				outputString(std::cout, data);
				std::cout << "\n";
				connection.client.send(XyzCpp::XyzUtils::encrypt(data, connection.secret), itype);
			}
		};

		client.onDisconnect = [&connection] (XyzCpp::XyzUtils::Error error) {
			std::cout << "[Info] Client disconnected: " << error.toString() << "\n";
		};

		client.read();
	};

	// Keep the program alive
	char c;
	while (true) {
		std::cin.get(c);
	}
}
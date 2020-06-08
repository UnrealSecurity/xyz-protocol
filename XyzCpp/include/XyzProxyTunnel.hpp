#pragma once

#include <vector>
#include <functional>
#include <optional>

#include "XyzClient.hpp"
#include "XyzServer.hpp"

namespace XyzCpp {
	class XyzProxyTunnel {
		protected:

		// this is unfortunate to have to wrap this in an optional
		std::optional<XyzServer> server;

		int listen_port;
		std::string forward_host;
		int forward_port;

		std::vector<XyzClient> backends;

		public:

		enum class Source {
			Client,
			Backend
		};

		std::function<void(XyzClient&, Source)> onConnect;
		std::function<void(XyzClient&, Source)> onDisconnect;
		std::function<void(XyzClient&, std::vector<std::byte>, int, Source)> onMessage;

		explicit XyzProxyTunnel (int t_listen_port, std::string t_forward_host, int t_forward_port) :
			listen_port(t_listen_port), forward_host(t_forward_host), forward_port(t_forward_port) {}

		void listen () {
			server.emplace(listen_port);
			server->onConnect = [this] (XyzClient& client) {
				this->invokeOnConnect(client, Source::Client);

				backends.push_back(XyzClient());
				XyzClient& backend = backends.at(backends.size() - 1);
				backend.onConnect = [this, &backend] () {
					this->invokeOnConnect(backend, Source::Backend);
				};
				backend.onDisconnect = [this, &client, &backend] () {
					client.disconnect();
					this->invokeOnDisconnect(backend, Source::Backend);
				};
				backend.onMessage = [this, &client, &backend] (std::vector<std::byte> bytes, int type) {
					client.send(bytes, type);
					this->invokeOnMessage(backend, bytes, type, Source::Backend);
				};

				backend.connect(forward_host, forward_port);

				client.onDisconnect = [this, &client, &backend] () {
					backend.disconnect();
					this->invokeOnDisconnect(client, Source::Client);
				};
				client.onMessage = [this, &client, &backend] (std::vector<std::byte> bytes, int type) {
					backend.send(bytes, type);
					this->invokeOnMessage(client, bytes, type, Source::Client);
				};
			};
		}

		protected:

		void invokeOnConnect (XyzClient& client, Source source) {
			if (onConnect) {
				onConnect(client, source);
			}
		}

		void invokeOnDisconnect (XyzClient& client, Source source) {
			if (onDisconnect) {
				onDisconnect(client, source);
			}
		}

		void invokeOnMessage (XyzClient& client, std::vector<std::byte> message, int type, Source source) {
			if (onMessage) {
				onMessage(client, message, type, source);
			}
		}
	};
}
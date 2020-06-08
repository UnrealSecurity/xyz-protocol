#pragma once

#include <boost/asio.hpp>

#include <functional>
#include <iostream>

#include "XyzMessageDecoder.hpp"
#include "XyzMessage.hpp"
#include "XyzMessageBuilder.hpp"

namespace XyzCpp {
	class XyzUdpClient {
		protected:
		std::shared_ptr<boost::asio::io_context> io_context;
		boost::asio::ip::udp::socket socket;
		boost::asio::ip::udp::endpoint endpoint;

		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;

		public:

		using Endpoint = boost::asio::ip::udp::endpoint;

		static Endpoint getEndpoint (std::string address, int port) {
			return Endpoint(boost::asio::ip::make_address(address), port);
		}

		static Endpoint getEndpoint (int port) {
			// no address means it listens on any interface
			return getEndpoint("0.0.0.0", port);
		}

		std::function<void(Endpoint sender_endpoint, std::vector<std::byte> bytes, int type)> onMessage;

		std::thread t;

		explicit XyzUdpClient () : io_context(std::make_shared<boost::asio::io_context>()), socket(*io_context), work(boost::asio::make_work_guard(*io_context)) {}
		// no constructor which takes address and port, as should we behave as a server or a client?
		//explicit XyzUdpClient (std::string address, int port) : io_context(std::make_shared<boost::asio::io_context>()), socket(*io_context) {
		//	bind(address, port);
		//}

		/// Required to be run manually if you are a client
		void start () {
			t = std::thread([this] () {
				std::cout << "Thread started\n";
				this->io_context->run();
				std::cout << "thread done\n";
			});
		}

		void open (Endpoint t_endpoint) {
			socket.open(t_endpoint.protocol());
		}

		/// Bind the server to this address and port
		/// If you're a client, then use get endpoint to compute the server endpoint and pass that into send functions
		void bind (std::string address, int port) {
			endpoint = getEndpoint(address, port);
			open(endpoint);
			socket.bind(endpoint);
			start();
			receiveData();
		}

		void bind (int port) {
			endpoint = getEndpoint(port);
			socket.bind(endpoint);
			start();
			receiveData();
		}

		void connect (Endpoint t_endpoint) {
			endpoint = t_endpoint;
			open(endpoint);
			socket.connect(endpoint);
			start();
			receiveData();
		}

		void close () {
			socket.close();
		}

		void send (const std::vector<std::byte>& data, int type=0) {
			send(data.data(), data.size(), type);
		}

		void send (const std::string& data, int type=0) {
			send(reinterpret_cast<const std::byte*>(data.data()), data.size());
		}

		void send (const std::vector<XyzMessage>& messages, int type=0) {
			XyzMessageBuilder builder;
			for (const XyzMessage& message : messages) {
				builder.add(message);
			}
			std::vector<std::byte> data = builder.toVector();
			send(data, type);
		}

		void send (const XyzMessage& message, int type=0) {
			XyzMessageBuilder builder;
			std::vector<std::byte> payload = builder.add(message).toVector();
			send(payload, type);
		}

		void send (const std::byte* data, size_t length, int type=0) {
			try {
				XyzMessageBuilder builder;
				std::vector<std::byte> payload = builder.add(XyzUtils::deflate(data, length)).toVector();
				socket.async_send(boost::asio::buffer(payload), [] (const boost::system::error_code& ec, size_t bytes_sent) {});
			} catch (...) {} // TODO: handle this
		}

		void send (Endpoint send_to, const std::vector<std::byte>& data, int type=0) {
			send(send_to, data.data(), data.size(), type);
		}

		void send (Endpoint send_to, const std::string& data, int type=0) {
			send(send_to, reinterpret_cast<const std::byte*>(data.data()), data.size());
		}

		void send (Endpoint send_to, const std::vector<XyzMessage>& messages, int type=0) {
			XyzMessageBuilder builder;
			for (const XyzMessage& message : messages) {
				builder.add(message);
			}
			std::vector<std::byte> data = builder.toVector();
			send(send_to, data, type);
		}

		void send (Endpoint send_to, const XyzMessage& message, int type=0) {
			XyzMessageBuilder builder;
			std::vector<std::byte> payload = builder.add(message).toVector();
			send(send_to, payload, type);
		}

		void send (Endpoint send_to, const std::byte* data, size_t length, int type=0) {
			try {
				XyzMessageBuilder builder;
				std::vector<std::byte> payload = builder
					.add(XyzUtils::deflate(data, length))
					.toVector();
				socket.async_send_to(boost::asio::buffer(payload), send_to, [] (const boost::system::error_code& ec, size_t bytes_sent) {});
			} catch (...) {} // TODO: handle this
		}

		void sendSync (Endpoint send_to, const std::vector<std::byte>& data, int type=0) {
			sendSync(send_to, data.data(), data.size(), type);
		}

		void sendSync (Endpoint send_to, const std::string& data, int type=0) {
			sendSync(send_to, reinterpret_cast<const std::byte*>(data.data()), data.size(), type);
		}

		void sendSync (Endpoint send_to, const std::vector<XyzMessage>& messages, int type=0) {
			XyzMessageBuilder builder;
			for (const XyzMessage& message : messages) {
				builder.add(message);
			}
			std::vector<std::byte> payload = builder.toVector();
			sendSync(send_to, payload, type);
		}

		void sendSync (Endpoint send_to, const XyzMessage& message, int type=0) {
			XyzMessageBuilder builder;
			std::vector<std::byte> payload = builder.add(message).toVector();
			sendSync(send_to, payload, type);
		}

		void sendSync (Endpoint send_to, const std::byte* data, size_t length, int type=0) {
			try {
				XyzMessageBuilder builder;
				std::vector<std::byte> payload = builder
					.add(XyzUtils::deflate(data, length))
					.toVector();
				socket.send_to(boost::asio::buffer(payload), send_to);
			} catch (...) {} // TODO: handle this
		}

		protected:

		void handleReceivedMessages (Endpoint sender_endpoint, std::vector<std::byte> data) {
			// how do we get the endpoint we got the data from?
			std::vector<XyzMessage> messages = XyzMessageDecoder(data).decode();

			for (XyzMessage& message : messages) {
				invokeOnMessage(sender_endpoint, XyzUtils::inflate(message.getBytes()), message.type);
			}
		}

		void receiveData () {
			// shared ptr because it needs to live
			std::shared_ptr<std::vector<std::byte>> data = std::make_shared<std::vector<std::byte>>();
			data->resize(65536);

			Endpoint sender_endpoint;
			socket.async_receive_from(boost::asio::buffer(*data), sender_endpoint, [this, data, &sender_endpoint] (boost::system::error_code ec, size_t bytes_received) {
				std::cout << "receiveData: " << ec << "\n";
				if (!ec && bytes_received > 0) {
					std::vector<std::byte> data_m = std::move(*data);
					data_m.resize(bytes_received);
					this->handleReceivedMessages(sender_endpoint, std::move(data_m));
				}

				this->receiveData();
			});
		}

		void invokeOnMessage (Endpoint sender_endpoint, std::vector<std::byte> bytes, int type) {
			if (onMessage) {
				onMessage(sender_endpoint, bytes, type);
			}
		}
	};
}
#pragma once

#define BOOST_ASIO_HAS_MOVE

#include <vector>
#include <functional>

#include <boost/asio.hpp>

#include "XyzUtils.hpp"
#include "XyzMessage.hpp"
#include "XyzMessageBuilder.hpp"

#include <iostream>

namespace XyzCpp {
	class TcpClient {
		protected:

		// Has to be made shared so it can be 'moved'..
		std::shared_ptr<boost::asio::io_context> io_context;
		boost::asio::ip::tcp::resolver resolver;
		boost::asio::ip::tcp::socket socket;
		// we have to create this work instance so that the io_context.run() does not exit
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;

		public:
		std::thread io_thread;

		// unsure if I really need resolver or work when we're being supplied the socket
		/// This constructor is made primarily for servers that are receiving a connection
		explicit TcpClient (boost::asio::ip::tcp::socket&& t_socket) : io_context(std::make_shared<boost::asio::io_context>()), resolver(*io_context), socket(std::move(t_socket)), work(boost::asio::make_work_guard(*io_context)) {}
		explicit TcpClient () : io_context(std::make_shared<boost::asio::io_context>()), resolver(*io_context), socket(*io_context), work(boost::asio::make_work_guard(*io_context)) {}

		/// Connect to the [host] on [port]
		/// Can throw a boost::system::system_error
		void connect (std::string host, int port) {
			// Note: in C#-version TCPClient only allows port to be between 0 and 65535

			boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
			// non-async connection, see: original
			boost::asio::connect(socket, endpoints);

			io_thread = std::thread([this] () {
				this->io_context->run();
			});
		}

		/// Returns whether the socket is still opened.
		bool isConnected () {
			return socket.is_open();
		}

		/// Close the socket.
		void close () {
			socket.close();
		}

		// These are templated because the function they're calling are templated. The templates could be more restrictive,
		// but most (all?) of the time they're only being called internally.

		/// Read data asynchronously from the socket with an arbitrary buffer & callback.
		template<typename B, typename F>
		void readAsync (B buffer, F callback) {
			boost::asio::async_read(socket, buffer, callback);
		}

		/// Write data asynchronously to the socket with an arbitrary buffer & callback
		template<typename B, typename F>
		void writeAsync (B buffer, F callback) {
			boost::asio::async_write(socket, buffer, callback);
		}

		/// Write data synchronously to the socket with an arbitrary buffer & callback
		template<typename B>
		void writeSync (B buffer) {
			boost::asio::write(socket, buffer);
		}
	};

	/// A TCP client for the XyzClient protocol. It would connect to a server that speaks this protocol.
	class XyzClient {
		public:
		/// The # of bytes the length takes up. A uint32_t
		static constexpr size_t length_size = 4;
		/// The # of bytes the type takes up. A single byte.
		/// (Though, the original source always converts it to an int..)
		static constexpr size_t type_size = 1;

		protected:
		TcpClient client;

		struct TempMessage {
			/// Buffer for storing length and then type before turning them into integers.
			std::array<std::byte, 4> buf;

			size_t length = 0;
			int type = 0;
			std::vector<std::byte> message;
		};

		public:
		
		std::function<void(void)> onConnect;
		std::function<void(XyzUtils::Error)> onDisconnect;
		std::function<void(std::vector<std::byte>, int)> onMessage;

		/// Default constructor. Call [connect] to connect to a specific server.
		/// Set onConnect, onDisconnect, and/or onMessage handlers
		explicit XyzClient () {}
		/// Note: this could cause issues, since it will fail/succeed to connect before you can apply the callbacks.
		explicit XyzClient (std::string host, int port) {
			connect(host, port);
		}
		explicit XyzClient (TcpClient&& t_client) : client(std::move(t_client)) {}

		/// Read a message from server
		/// Note: this is ran automatically (repeatedly) on-connect, so you likely don't need to call this
		void read () {
			readLength();
		}

		void disconnect (bool forced=false) {
			if (forced || client.isConnected()) {
				client.close();
				invokeOnDisconnect(XyzUtils::Error(XyzUtils::Error::Unknown{}));
			}
		}

		/// Connect to the [host] on [port].
		/// Calls [onConnect] on success
		/// Calls [onDisconnect] on any errors.
		bool connect (std::string host, int port) {
			try {
				client.connect(host, port);

				invokeOnConnect();

				read();
				return true;
			} catch (...) {
				invokeOnDisconnect(XyzUtils::Error(std::current_exception()));
			}
			return false;
		}

		bool connected () {
			// TODO: check for differences between this and original
			return client.isConnected();
		}

		void send (const std::vector<std::byte>& data, int type=0) {
			send(data.data(), data.size(), type);
		}

		void send (const std::string& data, int type=0) {
			send(reinterpret_cast<const std::byte*>(data.data()), data.size(), type);
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

		/// Send data synchronously
		void send (const std::byte* data, size_t length, int type=0) {
			try {
				std::vector<std::byte> payload = XyzMessageBuilder()
					.add(XyzUtils::deflate(data, length), type)
					.toVector();

				client.writeAsync(boost::asio::buffer(payload), [this] (boost::system::error_code ec, size_t length) {
					if (ec) {
						this->handleError(XyzUtils::Error(ec));
						return;
					}
				});
			} catch (...) {} // TODO: handle error?
		}

		void sendSync (std::vector<std::byte>& data, int type=0) {
			sendSync(data.data(), data.size());
		}

		void sendSync (const std::string& data, int type=0) {
			sendSync(reinterpret_cast<const std::byte*>(data.data()), data.size(), type);
		}

		void sendSync (const std::vector<XyzMessage>& messages, int type=0) {
			XyzMessageBuilder builder;
			for (const XyzMessage& message : messages) {
				builder.add(message);
			}
			std::vector<std::byte> data = builder.toVector();
			sendSync(data, type);
		}

		void sendSync (const XyzMessage& message, int type=0) {
			XyzMessageBuilder builder;
			std::vector<std::byte> payload = builder.add(message).toVector();
			sendSync(payload, type);
		}

		/// Send data synchronously as a message (compressed)
		void sendSync (const std::byte* data, size_t length, int type=0) {
			try {
				std::vector<std::byte> payload = XyzMessageBuilder()
					.add(XyzUtils::deflate(data, length))
					.toVector();

				client.writeSync(boost::asio::buffer(payload));
			} catch (...) {}
		}


		private:

		void readLength () {
			/// Note: this has to be shared instead of merely a unique ptr because we would then want to move it into the lambda to extend it's lifetime
			/// but then our lambda has move-only data in it, which makes it non-copyable, and internally the boost uses std::function (or something similar)
			/// which requires the lambda to be Copyable.
			std::shared_ptr<TempMessage> temp_message = std::make_shared<TempMessage>();
			client.readAsync(boost::asio::buffer(temp_message->buf.data(), length_size), [this, temp_message] (boost::system::error_code ec, size_t received_length) {
				this->receivedLength(ec, received_length, temp_message);
			});
		}

		void receivedLength (boost::system::error_code ec, size_t received_length, std::shared_ptr<TempMessage> temp_message) {
			if (ec) {
				handleError(XyzUtils::Error(ec));
				return;
			}

			try {
				// Fail if there's not enough bytes for a four byte integer (BitConverter.toInt32 does this)
				// (Though, we're using an array, so it's always filled, so we used the received_length variable to know what values are valid)
				if (received_length < 4) {
					throw XyzUtils::InsufficientBytes("For reading four bytes for message length");
				}

				// Convert bytes read into temp buffer into a uint32_t
				std::array<std::byte, 4>& buf = temp_message->buf;
				temp_message->length = XyzUtils::detail::asU32(buf.at(3), buf.at(2), buf.at(1), buf.at(0));

				// Resize the buffer to be able to contain the message
				temp_message->message.resize(temp_message->length);

				// Ask for the type byte
				client.readAsync(boost::asio::buffer(buf.data(), type_size), [this, temp_message] (boost::system::error_code ec, size_t t_received_length) {
					this->receivedType(ec, t_received_length, temp_message);
				});
			} catch (...) {
				handleError(XyzUtils::Error(std::current_exception()));
			}
		}

		void receivedType (boost::system::error_code ec, size_t received_length, std::shared_ptr<TempMessage> temp_message) {
			if (ec) {
				handleError(XyzUtils::Error(ec));
				return;
			}

			try {
				// Fail if not enough bytes for the byte of [type]
				if (received_length < 1) {
					throw XyzUtils::InsufficientBytes("For reading single byte for message type");
				}

				temp_message->type = static_cast<int>(temp_message->buf.at(0));

				client.readAsync(boost::asio::buffer(temp_message->message.data(), temp_message->length), [this, temp_message] (boost::system::error_code ec, size_t t_received_length) {
					this->receivedMessage(ec, t_received_length, temp_message);
				});
			} catch (...) {
				handleError(XyzUtils::Error(std::current_exception()));
			}
		}

		void receivedMessage (boost::system::error_code ec, size_t received_length, std::shared_ptr<TempMessage> temp_message) {
			if (ec) {
				handleError(XyzUtils::Error(ec));
				return;
			}

			try {
				// the deflated message is contained within message
				std::vector<std::byte> deflated = XyzUtils::inflate(temp_message->message);
				invokeOnMessage(std::move(deflated), temp_message->type);

				readLength();
			} catch (...) {
				handleError(XyzUtils::Error(std::current_exception()));
			}
		}

		void handleError (XyzUtils::Error error) {
			invokeOnDisconnect(error);
		}

		void invokeOnConnect () {
			if (onConnect) {
				onConnect();
			}
		}

		void invokeOnDisconnect (XyzUtils::Error error) {
			if (onDisconnect) {
				onDisconnect(error);
			}
		}

		void invokeOnMessage (std::vector<std::byte> data, int type) {
			if (onMessage) {
				onMessage(std::move(data), type);
			}
		}
	};
}
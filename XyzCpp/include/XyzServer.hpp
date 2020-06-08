#pragma once

#define BOOST_ASIO_HAS_MOVE

#include <boost/asio.hpp>

#include <vector>
#include <functional>
#include <memory>

#include "XyzUtils.hpp"
#include "XyzClient.hpp"

namespace XyzCpp {
	class XyzServer {
		protected:

		std::shared_ptr<boost::asio::io_context> io_context;
		boost::asio::ip::tcp::acceptor acceptor;

		// we have to create this work instance so that the io_context.run() does not exit
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;

		// note that there may be invalid/dead clients in this
		std::vector<XyzClient> sockets;


		static boost::asio::ip::tcp::endpoint getEndpoint (std::string address, int port) {
			return boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address), port);
		}

		static boost::asio::ip::tcp::endpoint getEndpoint (int port) {
			// no address means it listens on any interface
			return getEndpoint("0.0.0.0", port);
		}

		public:

		std::thread io_thread;

		std::function<void(XyzClient& client)> onConnect;

		explicit XyzServer (int port) :
			io_context(std::make_shared<boost::asio::io_context>()), acceptor(*io_context, getEndpoint(port)), work(boost::asio::make_work_guard(*io_context)) {
			acceptConnection();
			startIOThread();
		}

		explicit XyzServer (std::string address, int port) :
			io_context(std::make_shared<boost::asio::io_context>()), acceptor(*io_context, getEndpoint(address, port)), work(boost::asio::make_work_guard(*io_context)) {
			acceptConnection();
			startIOThread();
		}

		protected:

		void acceptConnection () {
			acceptor.async_accept([this] (boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
				// ignore socket if there was an error
				if (!ec) {
					sockets.push_back(XyzClient(TcpClient(std::move(socket))));
					invokeOnConnect(sockets.at(sockets.size() - 1));
				}

				this->acceptConnection();
			});
		}

		void startIOThread () {
			io_thread = std::thread([this] () {
				this->io_context->run();
			});
		}

		void invokeOnConnect (XyzClient& client) {
			if (onConnect) {
				onConnect(client);
			}
		}
	};
}
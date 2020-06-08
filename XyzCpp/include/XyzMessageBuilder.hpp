#pragma once

#include <vector>
#include <string>

#include "XyzUtils.hpp"
#include "XyzMessage.hpp"

namespace XyzCpp {
	class XyzMessageBuilder {
		std::vector<std::byte> data;

		public:

		XyzMessageBuilder () {}

		/// Note: this returns the internally held data, so this builder is invalidated.
		std::vector<std::byte> toVector () {
			return std::move(data);
		}

		XyzMessageBuilder& add (const std::vector<std::byte>& message, int type=0) {
			return add(message.data(), message.size(), type);
		}

		XyzMessageBuilder& add (const std::string& message, int type=0) {
			return add(reinterpret_cast<const std::byte*>(message.data()), message.size(), type);
		}

		XyzMessageBuilder& add (uint64_t number, int type=0) {
			std::array<std::byte, 8> bytes = XyzUtils::detail::fromU64LE(number);
			return add(bytes.data(), bytes.size(), type);
		}

		XyzMessageBuilder& add (uint32_t number, int type=0) {
			std::array<std::byte, 4> bytes = XyzUtils::detail::fromU32LE(number);
			return add(bytes.data(), bytes.size(), type);
		}

		XyzMessageBuilder& add (const XyzMessage& message) {
			return add(message.getBytes(), message.type);
		}

		XyzMessageBuilder& add (const std::vector<bool>& values, int type=0) {
			std::vector<std::byte> bytes;
			bytes.reserve(values.size());
			for (bool value : values) {
				bytes.push_back(std::byte(value));
			}

			return add(bytes, type);
		}

		XyzMessageBuilder& add (const std::vector<uint32_t>& values, int type=0) {
			std::vector<std::byte> bytes;
			bytes.reserve(values.size() * sizeof(uint32_t));

			for (uint32_t value : values) {
				XyzUtils::detail::pushBytes(bytes, XyzUtils::detail::fromU32LE(value));
			}

			return add(bytes, type);
		}

		XyzMessageBuilder& add (const std::vector<uint64_t>& values, int type=0) {
			std::vector<std::byte> bytes;
			bytes.reserve(values.size() * sizeof(uint64_t));

			for (uint64_t value : values) {
				XyzUtils::detail::pushBytes(bytes, XyzUtils::detail::fromU64LE(value));
			}

			return add(bytes, type);
		}


		private:

		XyzMessageBuilder& add (const std::byte* msg, uint32_t length, int type=0) {
			XyzUtils::detail::pushBytes(data, XyzUtils::detail::fromU32LE(length));

			data.push_back(static_cast<std::byte>(type));

			for (uint32_t i = 0; i < length; i++) {
				data.push_back(msg[i]);
			}

			return *this;
		}
	};
}
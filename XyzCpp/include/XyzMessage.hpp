#pragma once

#include <vector>

#include "XyzUtils.hpp"

namespace XyzCpp {
	class XyzMessage {
		protected:
		std::vector<std::byte> data;

		public:

		int type = 0;

		explicit XyzMessage (std::vector<std::byte> t_data, int t_type=0) : data(std::move(t_data)), type(t_type) {}

		explicit XyzMessage (std::vector<bool> t_data, int t_type=0) {
			data.reserve(t_data.size());

			for (bool value : t_data) {
				data.push_back(std::byte(value));
			}

			type = t_type;
		}

		explicit XyzMessage (uint32_t message, int t_type=0) {
			XyzUtils::detail::pushBytes(data, XyzUtils::detail::fromU32LE(message));
			type = t_type;
		}

		explicit XyzMessage (uint64_t message, int t_type=0) {
			XyzUtils::detail::pushBytes(data, XyzUtils::detail::fromU64LE(message));
			type = t_type;
		}

		explicit XyzMessage (const std::string& message, int t_type=0) {
			for (char chr : message) {
				data.push_back(std::byte(chr));
			}
			data.push_back(std::byte(0x00)); // null-terminated

			type = t_type;
		}

		explicit XyzMessage (const std::vector<uint32_t>& message, int t_type=0) {
			for (uint32_t value : message) {
				XyzUtils::detail::pushBytes(data, XyzUtils::detail::fromU32LE(value));
			}

			type = t_type;
		}

		explicit XyzMessage (const std::vector<uint64_t>& message, int t_type=0) {
			for (uint64_t value : message) {
				XyzUtils::detail::pushBytes(data, XyzUtils::detail::fromU64LE(value));
			}

			type = t_type;
		}

		const std::vector<std::byte>& getBytes () const {
			return data;
		}

		int getType () const {
			return type;
		}

		/// Reads a u32 at [i]
		/// Throws an error if there isn't enough bytes (4) for a U32.
		uint32_t asU32 (size_t i=0) const {
			// 8 bytes [0 1 2 3 4 5 6 7]
			// at position 4: [4 5 6 7], valid
			// at position 5: [5 6 7], invalid
			if (data.size() <= (i + 3)) {
				throw XyzUtils::InsufficientBytes("For reading as uint32");
			}
			return XyzUtils::detail::asU32(data.at(i+3), data.at(i+2), data.at(i+1), data.at(i));
		}

		/// Reads a u64 at [i]
		/// Throws an error if there isn't enough bytes (8) for a U64
		uint64_t asU64 (size_t i=0) const {
			if (data.size() <= (i + 7)) {
				throw XyzUtils::InsufficientBytes("For reading as uint64");
			}

			return XyzUtils::detail::asU64(
				data.at(i+7), data.at(i+6), data.at(i+5), data.at(i+4), data.at(i+3), data.at(i+2), data.at(i+1), data.at(i)
			);
		}

		/// Warning: potentially expensive operation
		/// Note: this isn't completely accurate to the original, as I don't know any way of checking for utf8 validity.
		std::string asString () const {
			if (data.size() == 0) {
				return "";
			}

			std::string result;
			result.reserve(data.size());

			// -1 to avoid the ending 0x00
			for (size_t i = 0; i < data.size() - 1; i++) {
				result += static_cast<char>(data[i]);
			}

			return result;
		}

		/// Warning: potentially expensive operation
		std::vector<std::string> asStrings () const {
			std::vector<std::string> result;

			std::string current;
			bool consuming = true;
			for (std::byte value : data) {
				if (value == std::byte(0x00)) {
					result.push_back(std::move(current));
					current = std::string();
					consuming = false;
				} else {
					consuming = true;
					current += static_cast<char>(value);
				}
			}

			// If we were parsing a string, but then it ended before we found the null byte, add it anyway.
			if (consuming) {
				result.push_back(std::move(current));
			}

			return result;
		}

		/// Warning potentially expensive operation
		std::vector<bool> asBooleans () const {
			std::vector<bool> result;
			result.reserve(data.size());

			for (std::byte value : data) {
				result.push_back(static_cast<bool>(value));
			}

			return result;
		}

		/// Warning potentially expensive operation
		/// May throw an error if it tries reading a u32 and the remaining data is insufficient
		std::vector<uint32_t> asU32s () const {
			std::vector<uint32_t> result;
			result.reserve(data.size() / sizeof(uint32_t));

			for (size_t i = 0; i < data.size(); i += sizeof(uint32_t)) {
				result.push_back(asU32(i));
			}

			return result;
		}

		/// Warning potentially expensive operation
		/// May throw an error if it tries reading a u32 and the remaining data is insufficient
		std::vector<uint64_t> asU64s () const {
			std::vector<uint64_t> result;
			result.reserve(data.size() / sizeof(uint64_t));

			for (size_t i = 0; i < data.size(); i += sizeof(uint64_t)) {
				result.push_back(asU64(i));
			}

			return result;
		}
	};
}
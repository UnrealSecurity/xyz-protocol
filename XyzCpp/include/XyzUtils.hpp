#pragma once

#include <vector>
#include <string>
#include <array>
#include <variant>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
//#define BOOST_IOSTREAMS_NO_LIB
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

namespace XyzCpp {
	namespace XyzUtils {
		namespace detail {
			// Utility functions that aren't officially a part of xyz

			static uint8_t asU8 (std::byte value) {
				return static_cast<uint8_t>(value);
			}

			// Note: these are all little-endian
			static uint16_t asU16 (std::byte hi, std::byte lo) {
				return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo);
			}

			static uint32_t asU32 (std::byte hihi, std::byte lohi, std::byte hilo, std::byte lolo) {
				return (static_cast<uint32_t>(asU16(hihi, lohi)) << 16) | static_cast<uint32_t>(asU16(hilo, lolo));
			}

			static uint64_t asU64 (std::byte a, std::byte b, std::byte c, std::byte d, std::byte e, std::byte f, std::byte g, std::byte h) {
				return (static_cast<uint64_t>(asU32(a, b, c, d)) << 32) |
					static_cast<uint64_t>(asU32(e, f, g, h));
			}

			


			static std::array<std::byte, 4> fromU32LE (uint32_t v) {
				return {
					std::byte(v & 0x000000FF),
					std::byte((v & 0x0000FF00) >> 8),
					std::byte((v & 0x00FF0000) >> 16),
					std::byte((v & 0xFF000000) >> 24),
				};
			}

			static std::array<std::byte, 8> fromU64LE (uint64_t v) {
				return {
					std::byte(v & 0x00000000000000FF),
					std::byte((v & 0x000000000000FF00) >> 8),
					std::byte((v & 0x0000000000FF0000) >> 16),
					std::byte((v & 0x00000000FF000000) >> 24),
					std::byte((v & 0x000000FF00000000) >> 32),
					std::byte((v & 0x0000FF0000000000) >> 40),
					std::byte((v & 0x00FF000000000000) >> 48),
					std::byte((v & 0xFF00000000000000) >> 56)
				};
			}

			/// only valid for v <= 15
			static char nibbleToHex (std::byte v) {
				if (v > std::byte(9)) {
					return (static_cast<char>(v) - 10) + 'a';
				} else {
					return static_cast<char>(v) + '0';
				}
			}

			static std::array<char, 2> toHex (std::byte v) {
				return {
					nibbleToHex((v & std::byte(0xF0)) >> 4),
					nibbleToHex(v & std::byte(0x0F))
				};
			}

			template<size_t N>
			static void pushBytes (std::vector<std::byte>& data, const std::array<std::byte, N>& bytes) {
				for (std::byte value : bytes) {
					data.push_back(value);
				}
			}

			// had to define custom back_insertor (pretty simple), since iostreams don't convert char/unsigned char into std::byte..
			struct byte_insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
				std::vector<std::byte>* place;

				explicit byte_insert_iterator (std::vector<std::byte>& t_place) : place(std::addressof(t_place)) {}

				byte_insert_iterator& operator= (unsigned char& value) {
					place->push_back(std::byte(value));
					return *this;
				}
				byte_insert_iterator& operator= (unsigned char&& value) {
					place->push_back(std::byte(value));
					return *this;
				}

				byte_insert_iterator& operator= (std::byte& value) {
					place->push_back(value);
					return *this;
				}

				byte_insert_iterator& operator= (std::byte&& value) {
					place->push_back(value);
					return *this;
				}

				byte_insert_iterator& operator* () {
					return *this;
				}

				byte_insert_iterator& operator++ () {
					return *this;
				}

				byte_insert_iterator& operator++ (int) {
					return *this;
				}
			};

			static std::vector<std::byte> EVPBytes (const char* evp_name, size_t size, const std::byte* data, size_t length) {
				std::vector<std::byte> result;
				result.resize(size);


				// this isn't the most efficient method of getting it, but it works
				const EVP_MD* md = EVP_get_digestbyname(evp_name);
				if (md == nullptr) {
					throw std::runtime_error("Failed to get MD5 digest");
				}

				EVP_MD_CTX* context = EVP_MD_CTX_new();

				EVP_DigestInit_ex(context, md, nullptr);
				EVP_DigestUpdate(context, reinterpret_cast<const void*>(data), length);
				unsigned int output_size;
				EVP_DigestFinal_ex(context, reinterpret_cast<unsigned char*>(result.data()), &output_size);

				EVP_MD_CTX_free(context);

				result.resize(output_size);

				return result;
			}

			static std::string EVP (const char* evp_name, size_t size, const std::byte* data, size_t length) {
				std::vector<std::byte> result = EVPBytes(evp_name, size, data, length);

				std::string output;
				output.reserve(result.size() * 2); // 2 bytes for each character
				for (std::byte entry : result) {
					std::array<char, 2> hex = toHex(entry);
					output += hex[0];
					output += hex[1];
				}

				return output;
			}

			static const std::array<std::byte, 8> salt{std::byte(0x44), std::byte(0x12), std::byte(0x8), std::byte(0x19), std::byte(0x4D), std::byte(0x51), std::byte(0x1B), std::byte(0x9)};
			static constexpr size_t aes_secret_key_size = 32;
			static constexpr size_t aes_iv_size = 16; // initialization vector
			static constexpr size_t aes_key_size = aes_secret_key_size + aes_iv_size;
			static void initAES (const std::byte* key, size_t key_length, const std::byte* iv, size_t iv_length, EVP_CIPHER_CTX* cipher_context, bool enc) {
				EVP_CIPHER_CTX_init(cipher_context);
				EVP_CipherInit_ex(cipher_context, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key), reinterpret_cast<const unsigned char*>(iv), static_cast<int>(enc));
			}

			static std::pair<std::array<std::byte, aes_secret_key_size>, std::array<std::byte, aes_iv_size>> generatePBKDF2 (
				const std::byte* pass, size_t pass_length, const std::byte* salt, size_t salt_length, size_t iter_count) {
				std::array<std::byte, aes_key_size> key;

				int status = PKCS5_PBKDF2_HMAC_SHA1(
					// password
					reinterpret_cast<const char*>(pass), pass_length,
					// salt
					reinterpret_cast<const unsigned char*>(salt), salt_length,
					iter_count,
					// key, produces random data into it
					key.size(), reinterpret_cast<unsigned char*>(key.data())
				);

				if (status == 0) {
					throw std::runtime_error("PBKDF2 Failure");
				}

				std::array<std::byte, aes_secret_key_size> secret_key;
				std::copy(key.begin(), key.begin() + aes_secret_key_size, secret_key.begin());

				std::array<std::byte, aes_iv_size> iv;
				std::copy(key.begin() + aes_secret_key_size, key.end(), iv.begin());

				return std::make_pair(secret_key, iv);
			}

			/// [secret_key] size should be [aes_secret_key_size]
			/// [iv] size should be [aes_iv_size]
			static EVP_CIPHER_CTX* createEVPCipherContext (const std::byte* secret_key, const std::byte* iv, bool enc) {
				EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
				initAES(secret_key, aes_secret_key_size, iv, aes_iv_size, context, enc);
				return context;
			}

			// This is rather low?
			static constexpr size_t pk_iteration_count = 2;
			
		}

		// ==== Errors ====
		
		/// Thrown when there isn't enough bytes for an operation.
		struct InsufficientBytes : public std::runtime_error {
			explicit InsufficientBytes (std::string message) : std::runtime_error("Insufficient Bytes: " + message) {}
		};

		/// An strange struct to hold the error within it.
		/// This has to hold every possible exception that could be thrown, and copy them *properly*
		/// (if we just listened for a )
		struct Error {
			/// Unknown error tag, use this when you, well, don't have a good idea what the error is.
			struct Unknown {};
			std::variant<
				std::exception_ptr,
				boost::system::error_code,
				std::string,
				Unknown
			> error;

			Error (std::exception_ptr e_ptr) : error(e_ptr) {}
			Error (boost::system::error_code ec) : error(ec) {}
			Error (std::string str) : error(str) {}
			Error (Unknown u) : error (u) {}
			
			std::string toString () const {
				if (std::holds_alternative<std::exception_ptr>(error)) {
					try {
						// We have to rethrow the exception if we want to access it again.
						std::rethrow_exception(std::get<std::exception_ptr>(error));
					} catch (std::exception& e) {
						// We know how to handle a std::exception, so we get it's error string
						return e.what();
					} catch (int i) {
						// Perhaps some status code? This isn't likely.
						return std::to_string(i);
					} catch (...) {
						// No clue what the error is.
						return "Unknown Error";
					}
				} else if (std::holds_alternative<boost::system::error_code>(error)) {
					boost::system::error_code ec = std::get<boost::system::error_code>(error);
					return ec.message();
				} else if (std::holds_alternative<std::string>(error)) {
            		return std::get<std::string>(error);
				} else {
					return "Unknown Error";
				}
			}
		};

		// ==== Deflate / Inflate ====
		// Compression method used on all messages

		static std::vector<std::byte> deflate (const std::byte* bytes, size_t length) {
			std::vector<std::byte> result;
			{
				boost::iostreams::filtering_ostream os;
				boost::iostreams::zlib_params params;
				params.noheader = true;
				os.push(boost::iostreams::zlib_compressor(params));
				os.push(detail::byte_insert_iterator(result));
				boost::iostreams::write(os, reinterpret_cast<const char*>(bytes), length);
			}
			return result;
		}

		static std::vector<std::byte> deflate (const std::vector<std::byte>& bytes) {
			return deflate(bytes.data(), bytes.size());
		}

		static std::vector<std::byte> inflate (const std::byte* bytes, size_t length) {
			std::vector<std::byte> result;

			{
				boost::iostreams::filtering_ostream os;
				boost::iostreams::zlib_params params;
				params.noheader = true;
				os.push(boost::iostreams::zlib_decompressor(params));
				os.push(detail::byte_insert_iterator(result));
				boost::iostreams::write(os, reinterpret_cast<const char*>(bytes), length);
			}

			return result;
		}

		std::vector<std::byte> inflate (const std::vector<std::byte>& bytes) {
			return inflate(bytes.data(), bytes.size());
		}

		// ==== Encrypt/Decrypt ====
		// Relatively simple encryption method that can be combined with XyzKeyExchange

		static std::vector<std::byte> encrypt (const std::byte* message, size_t length, const std::byte* pass, size_t pass_length) {
			auto [aes_secret_key, aes_iv] = detail::generatePBKDF2(
				pass, pass_length,
				detail::salt.data(), detail::salt.size(),
				detail::pk_iteration_count
			);

			EVP_CIPHER_CTX* encrypt_context = detail::createEVPCipherContext(aes_secret_key.data(), aes_iv.data(), true);

			const size_t block_size = EVP_CIPHER_CTX_block_size(encrypt_context);

			std::vector<std::byte> result;
			// https://crypto.stackexchange.com/questions/54017/can-we-calculate-aes-cbc-pkcs7-ciphertext-length-based-on-the-length-of-the-plai
			// probably don't need the + 1
			result.resize(length + (block_size - (length % block_size)) + 1);

			int out_length = result.size();
			if (!EVP_EncryptUpdate(encrypt_context, reinterpret_cast<unsigned char*>(result.data()), &out_length, reinterpret_cast<const unsigned char*>(message), length)) {
				throw std::runtime_error("Failed to encrypt update");
			}

			int out_length2 = result.size();
			if (!EVP_EncryptFinal_ex(encrypt_context, reinterpret_cast<unsigned char*>(result.data()) + out_length, &out_length2)) {
				throw std::runtime_error("Failed to encrypt final");
			}

			EVP_CIPHER_CTX_cleanup(encrypt_context);

			// resize to the actual amount of data that we encrypted
			result.resize(out_length + out_length2);

			return result;
		}

		static std::vector<std::byte> decrypt (const std::byte* message, size_t length, const std::byte* pass, size_t pass_length) {
			auto [aes_secret_key, aes_iv] = detail::generatePBKDF2(
				pass, pass_length,
				detail::salt.data(), detail::salt.size(),
				detail::pk_iteration_count
			);

			EVP_CIPHER_CTX* encrypt_context = detail::createEVPCipherContext(aes_secret_key.data(), aes_iv.data(), false);

			std::vector<std::byte> result;
			result.resize(length);

			int out_length = 0;
			EVP_DecryptUpdate(encrypt_context, reinterpret_cast<unsigned char*>(result.data()), &out_length, reinterpret_cast<const unsigned char*>(message), length);

			int out_length2 = 0;
			EVP_DecryptFinal_ex(encrypt_context, reinterpret_cast<unsigned char*>(result.data()) + out_length, &out_length2);

			EVP_CIPHER_CTX_cleanup(encrypt_context);

			result.resize(out_length + out_length2);

			return result;
		}

		// original-C# code calls 'pass' key, but that's confusing since PBKDF2/Rfc2898 produces a key, and it's officially called 'password'
		static std::vector<std::byte> encrypt (const std::vector<std::byte>& bytes, const std::vector<std::byte>& pass) {
			return encrypt(bytes.data(), bytes.size(), pass.data(), pass.size());
		}

		static std::vector<std::byte> decrypt (const std::vector<std::byte>& bytes, const std::vector<std::byte>& pass) {
			return decrypt(bytes.data(), bytes.size(), pass.data(), pass.size());
		}

		// ==== Simple Functions ====

		// == SHA256 (String) ==

		static std::string SHA256 (const std::byte* data, size_t length) {
			return detail::EVP("SHA256", 64, data, length);
		}

		static std::string SHA256 (const std::vector<std::byte>& input) {
			return SHA256(input.data(), input.size());
		}

		static std::string SHA256 (const std::string& input) {
			return SHA256(reinterpret_cast<const std::byte*>(input.data()), input.size());
		}

		// == SHA256 (Bytes) ==

		static std::vector<std::byte> SHA256Bytes (const std::byte* data, size_t length) {
			return detail::EVPBytes("SHA256", 64, data, length);
		}

		static std::vector<std::byte> SHA256Bytes (const std::vector<std::byte>& input) {
			return SHA256Bytes(input.data(), input.size());
		}

		static std::vector<std::byte> SHA256Bytes (const std::string& input) {
			return SHA256Bytes(reinterpret_cast<const std::byte*>(input.data()), input.size());
		}

		// == MD5 ==
		static std::string MD5 (const std::byte* data, size_t length) {
			return detail::EVP("MD5", 32, data, length);
		}

		static std::string MD5 (const std::vector<std::byte>& input) {
			return MD5(input.data(), input.size());
		}

		static std::string MD5 (const std::string& input) {
			return MD5(reinterpret_cast<const std::byte*>(input.data()), input.size());
		}
	}
}
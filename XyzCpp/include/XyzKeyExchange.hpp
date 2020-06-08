#pragma once

#include <vector>
#include <exception>
#include <array>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/ecdh.h>

#include "XyzUtils.hpp"

namespace XyzCpp {
	namespace detail {
		// The curve we're using, the code may have assumptions baked into it from this. (Ex: the coordinate size)
		static constexpr int NIDCurve = NID_secp521r1;

		// OpenSSL documentation for this is rather lacking, but after many hours of trial and error I found the seemingly working way to generate the keys
		// and then send them over the wire (you'd think they'd describe this in the documentation..)

		/// Generates the private key (which we can later extract the public key from)
		EVP_PKEY* generateKey () {
			EVP_PKEY_CTX* param_gen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);

			if (!EVP_PKEY_paramgen_init(param_gen_ctx)) {
				throw std::runtime_error("Could not init paramgen");
			}

			EVP_PKEY_CTX_set_ec_paramgen_curve_nid(param_gen_ctx, NIDCurve);

			EVP_PKEY* params = nullptr;
			EVP_PKEY_paramgen(param_gen_ctx, &params);


			EVP_PKEY_CTX* key_gen_ctx = EVP_PKEY_CTX_new(params, nullptr);

			if (!EVP_PKEY_keygen_init(key_gen_ctx)) {
				throw std::runtime_error("Failed to init keygen");
			}

			EVP_PKEY* key_pair = nullptr;
			if (!EVP_PKEY_keygen(key_gen_ctx, &key_pair)) {
				throw std::runtime_error("Failed to keygen");
			}

			EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(key_pair);

			const BIGNUM* priv_key = EC_KEY_get0_private_key(ec_key);

			const EC_POINT* pub_point = EC_KEY_get0_public_key(ec_key);

			BIGNUM* x = BN_new();
			BIGNUM* y = BN_new();

			EC_POINT_get_affine_coordinates_GFp(EC_GROUP_new_by_curve_name(NIDCurve), pub_point, x, y, nullptr);

			// If you wish to print out the generated key, then use this:
			//BN_print_fp(stdout, priv_key);
			// public x
			//BN_print_fp(stdout, x);
			// public y
			//BN_print_fp(stdout, y);



			// cleanup
			EVP_PKEY_CTX_free(param_gen_ctx);
			EVP_PKEY_CTX_free(key_gen_ctx);

			return key_pair;
		}

		/// Extracts the public key from a given private key
		EVP_PKEY* extractPublicKey (EVP_PKEY* private_key) {
			EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(private_key);
			const EC_POINT* ec_point = EC_KEY_get0_public_key(ec_key);

			EVP_PKEY* public_key = EVP_PKEY_new();

			EC_KEY* public_ec_key = EC_KEY_new_by_curve_name(NIDCurve);

			EC_KEY_set_public_key(public_ec_key, ec_point);

			EVP_PKEY_set1_EC_KEY(public_key, public_ec_key);

			EC_KEY_free(ec_key);

			return public_key;
		}

		/// Compute the shared secret from the peer's public key and your private key
		std::vector<std::byte> deriveShared (EVP_PKEY* public_key, EVP_PKEY* private_key) {
			std::vector<std::byte> shared;

			EVP_PKEY_CTX* derivation_ctx = EVP_PKEY_CTX_new(private_key, nullptr);

			EVP_PKEY_derive_init(derivation_ctx);

			EVP_PKEY_derive_set_peer(derivation_ctx, public_key);

			// Get the size needed for the shared secret
			size_t length = 0;
			if (EVP_PKEY_derive(derivation_ctx, nullptr, &length) != 1) {
				throw std::runtime_error("Failed to derive length");
			}

			shared.resize(length);

			// Derive the shared secret
			if (EVP_PKEY_derive(derivation_ctx, reinterpret_cast<unsigned char*>(shared.data()), &length) != 1) {
				throw std::runtime_error("Failed to derive");
			}

			// cleanup
			EVP_PKEY_CTX_free(derivation_ctx);

			return shared;
		}

		// might only work on public keys
		EVP_PKEY* convertBytesToKey (std::byte* data, size_t length) {
			const point_conversion_form_t form = POINT_CONVERSION_UNCOMPRESSED;

			EC_GROUP* ec_group = EC_GROUP_new_by_curve_name(NID_secp521r1); // hopefully this is correct

			EC_POINT* point = EC_POINT_new(ec_group);
			if (EC_POINT_oct2point(ec_group, point, reinterpret_cast<const unsigned char*>(data), length, nullptr) != 1) {
				throw std::runtime_error("Failed to run oct2point");
			}

			EC_KEY* ec_key = EC_KEY_new();
			EC_KEY_set_group(ec_key, ec_group);

			//if (!EC_KEY_generate_key(ec_key)) {
			//	throw std::runtime_error("Failed to generate key");
			//}

			if (EC_KEY_set_public_key(ec_key, point) != 1) {
				throw std::runtime_error("Failed to set public key");
			}

			EVP_PKEY* pkey = EVP_PKEY_new();
			if (EVP_PKEY_set1_EC_KEY(pkey, ec_key) != 1) {
				throw std::runtime_error("Failed to set pkey");
			}

			return pkey;
		}

		/// Converts the given key into bytes that can be sent over the wire (in ECK5 format, which is what the original uses)
		/// Takes a private key, and extracts the public key from it.
		std::vector<std::byte> convertKeyToBytes (EVP_PKEY* key) {
			// Our data should be uncompressed
			const point_conversion_form_t form = POINT_CONVERSION_UNCOMPRESSED;

			EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(key);
			const EC_POINT* point = EC_KEY_get0_public_key(ec_key);

			EC_GROUP* ec_group = EC_GROUP_new_by_curve_name(NID_secp521r1); // hopefully this is correct

			std::vector<std::byte> result;

			// get the length needed for the buffer
			size_t length = 0;
			length = EC_POINT_point2oct(ec_group, point, form, nullptr, length, nullptr);
			result.resize(length);

			// Turn the key into bytes (oct being short for octet, which is 8 bits which is just a more exact way of referring to a byte)
			size_t read_length = EC_POINT_point2oct(ec_group, point, form, reinterpret_cast<unsigned char*>(result.data()), length, nullptr);
			result.resize(read_length);

			// there's some weird signature on the key that we want to strip
			// 0x4 is uncompressed (0x02 or 0x03 is compressed, and then there's a hybrid option as well..)
			result.erase(result.begin()); // erase 0x4
			// ECK5 is the header of what the orignal library uses.
			// I had trouble finding much of anything on it beyond: https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/ns-bcrypt-bcrypt_ecckey_blob?redirectedfrom=MSDN
			// which gives info about it's structure (ah, windows, and it's love for storing data past the end of a structure)
			std::array<std::byte, 8> header = {std::byte('E'), std::byte('C'), std::byte('K'), std::byte('5')};
			// It doesn't explicitly mention whether it's always little endian, but if it is based on the receiving system then that would add a *huge* dose of complexity.
			// we get the length (0x42 = 66 bytes) which is the length of each part of the key. There's two parts X and Y, which we store (literally points on a massive curve)
			std::array<std::byte, 4> temp = XyzUtils::detail::fromU32LE(0x42);
			header[4] = temp[0];
			header[5] = temp[1];
			header[6] = temp[2];
			header[7] = temp[3];

			result.insert(result.begin(), header.begin(), header.end());
			return result;
		}
	/*
		// zero buffer to detect empty buffers
		static std::array<std::byte, 3 * NN_MAX_BYTE_LEN> zero;

		int ecdh_helper (const unsigned char* curve_name, std::byte* public_buffer, std::byte* other_public_buffer) {
			// projective point
			prj_pt Q;
			// equivalent affine point
			aff_pt Q_aff;
	

			const ec_str_params* curve_const_parameters;
			// libecc internal struct holding curve params
			ec_params curve_params;

			nn_t d;
			fp_t x;


			curve_const_parameters = ec_get_curve_params_by_name(curve_name, static_cast<uint8_t>(local_strnlen(curve_name, MAX_CURVE_NAME_LEN)) + 1);
			
			if (curve_const_parameters == nullptr) {
				throw std::runtime_error("Failed to get curve parameters.");
			}

			// map curve parameters to libecc internal rep
			import_params(&curve_params, curve_const_parameters);

			// init projective point with curve params
			prj_pt_init(&Q, &(curve_params.ec_curve));
			
			// gen ecdg parameters
			nn_init(d, 0);
			if (nn_get_random_mod(d, &(curve_params.ec_gen_order))) {
				throw std::runtime_error("Failed get rando mmod");
			}


		}*/
	}

	class XyzKeyExchange {
		EVP_PKEY* private_key = nullptr;
		EVP_PKEY* public_key = nullptr;

		EVP_PKEY* peer_public_key = nullptr;

		std::optional<std::vector<std::byte>> shared_secret;

		public:

		/// A class for creating a private and public key, receiving a remote-peer key, and generating a shared secret.
		explicit XyzKeyExchange () {
			private_key = detail::generateKey();
			public_key = detail::extractPublicKey(private_key);
		}

		/// Get your generated public key. This is what can be sent over to another client/server to facilitate more secure communication.
		std::vector<std::byte> getLocalPublicKey () {
			return detail::convertKeyToBytes(private_key);
		}

		/// Set the peer's public key. This is their public key which allows you to generate the shared-secret-key which you can use for
		/// more secure communication.
		void setRemotePublicKey (std::vector<std::byte> t_remote_public_key) {
			shared_secret = std::nullopt;

			// Remove the header of [ECK5][Key-part size]
			t_remote_public_key.erase(t_remote_public_key.begin() + 0, t_remote_public_key.begin() + 8);
			// Add the uncompressed byte that OpenSSL includes in the data
			t_remote_public_key.insert(t_remote_public_key.begin() + 0, std::byte(POINT_CONVERSION_UNCOMPRESSED));
			peer_public_key = detail::convertBytesToKey(t_remote_public_key.data(), t_remote_public_key.size());
		}

		/// Generate the shared secret key. This is what you would use for encrypting/decrypting data.
		std::vector<std::byte> getSharedSecretKey () {
			if (!shared_secret.has_value()) {
				std::vector<std::byte> data = detail::deriveShared(peer_public_key, private_key);
				shared_secret = XyzUtils::SHA256Bytes(data.data(), data.size());	
			}
			return shared_secret.value();
		}
	};
}
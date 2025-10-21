/******************************************************************************
 *  Copyright (c) 2025 The CRTC project authors . All Rights Reserved.
 *
 *  Please visit https://chensongpoixs.github.io for detail
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 ******************************************************************************/
 /*****************************************************************************
				   Author: chensong
				   date:  2025-10-14



 ******************************************************************************/
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/librtc/dtls.h"
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include "libmedia_transfer_protocol/librtc/srtp_session.h"
namespace libmedia_transfer_protocol {
	namespace libssl {


		namespace {
			/* Static methods for OpenSSL callbacks. */

			inline static int OnSslCertificateVerify(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/)
			{
				 

				// Always valid since DTLS certificates are self-signed.
				return 1;
			}

			inline static void OnSslInfo(const SSL* ssl, int where, int ret)
			{
				//DEBUG_EX_LOG("[where = %d][ret = %d]", where, ret);
				// SSL_set_ex_data(ssl_, 0, static_cast<void*>(this));

				static_cast<Dtls*>(SSL_get_ex_data(ssl, 0))->OnSslInfo(where, ret);
			}
		}

		DtlsCerts::DtlsCerts(){}
		DtlsCerts::~DtlsCerts()
		{
			//if (dtls_pkey_)
			//{
			//	EVP_PKEY_free(dtls_pkey_);
			//	dtls_pkey_ = nullptr;
			//}
			//
			//if (dtls_certs_)
			//{
			//	X509_free(dtls_certs_);
			//	dtls_certs_ = nullptr;
			//}
		}
		bool DtlsCerts::Init(const char * dtls_certificate_file  ,
			const char * dtls_private_key_file  )
		{ 
				//MS_DEBUG_TAG(info, "openssl version: \"%s\"", OpenSSL_version(OPENSSL_VERSION));
			LIBSSL_LOG(LS_INFO) << "openssl version:" << OpenSSL_version(OPENSSL_VERSION);
				// Initialize some crypto stuff.
			RAND_poll();
			if (!dtls_certificate_file || !dtls_private_key_file)
			{
				GenerateCertificateAndPrivateKey();
			}
			else
			{
				ReadCertificateAndPrivateKeyFromFiles(dtls_certificate_file, dtls_private_key_file);
			}
			// Create a global SSL_CTX.
			CreateSslCtx();

			// Generate certificate fingerprints.
			GenerateFingerprints();

			return true;
		}
		void DtlsCerts::Destroy()
		{
			if (private_key_)
			{
				EVP_PKEY_free(private_key_);
			}
			if (certificate_)
			{
				X509_free(certificate_);
			}
			if (ssl_ctx_)
			{
				SSL_CTX_free(ssl_ctx_);
			}
		}
		const std::vector<libssl::Fingerprint> &DtlsCerts::Fingerprints()const
		{
			return local_fingerprints_;
		}
		EVP_PKEY *DtlsCerts::GetPrivateKey()const
		{
			return private_key_;
		}
		 
		X509 * DtlsCerts::GetCertificate() const
		{
			return certificate_;
		}
		SSL_CTX * DtlsCerts::GetSslCtx() const
		{
			return ssl_ctx_;
		}
		uint32_t DtlsCerts::GenRandom()
		{
			std::mt19937 mt{ std::random_device{}() };
			return mt();
		}
		void DtlsCerts::CreateSslCtx()
		{
			std::string dtlsSrtpCryptoSuites;
			int ret;

			/* Set the global DTLS context. */

			// Both DTLS 1.0 and 1.2 (requires OpenSSL >= 1.1.0).
			ssl_ctx_ = SSL_CTX_new(DTLS_method());

			if (!ssl_ctx_)
			{
				//LOG_OPENSSL_ERROR("SSL_CTX_new() failed");

				LIBSSL_LOG_T_F(LS_ERROR) << "SSL_CTX_new() failed";
				goto error;
			}

			ret = SSL_CTX_use_certificate(ssl_ctx_, DtlsCerts::GetInstance().GetCertificate());

			if (ret == 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("SSL_CTX_use_certificate() failed");

				goto error;
			}

			ret = SSL_CTX_use_PrivateKey(ssl_ctx_, DtlsCerts::GetInstance().GetPrivateKey());

			if (ret == 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("SSL_CTX_use_PrivateKey() failed");

				goto error;
			}

			ret = SSL_CTX_check_private_key(ssl_ctx_);

			if (ret == 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("SSL_CTX_check_private_key() failed");

				goto error;
			}

			// Set options.
			SSL_CTX_set_options(
				ssl_ctx_,
				SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET | SSL_OP_SINGLE_ECDH_USE |
				SSL_OP_NO_QUERY_MTU);

			// Don't use sessions cache.
			SSL_CTX_set_session_cache_mode(ssl_ctx_, SSL_SESS_CACHE_OFF);

			// Read always as much into the buffer as possible.
			// NOTE: This is the default for DTLS, but a bug in non latest OpenSSL
			// versions makes this call required.
			SSL_CTX_set_read_ahead(ssl_ctx_, 1);

			SSL_CTX_set_verify_depth(ssl_ctx_, 4);

			// Require certificate from peer.
			//SSL_CTX_set_verify(
			//	ssl_context_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, onSslCertificateVerify);

			// Set SSL info callback.
			//SSL_CTX_set_info_callback(ssl_context_, onSslInfo);
			SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, OnSslCertificateVerify);
			//状态切换  回调用函数
			SSL_CTX_set_info_callback(ssl_ctx_, OnSslInfo);
			// Set ciphers.
			ret = SSL_CTX_set_cipher_list(
				ssl_ctx_, "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK");

			if (ret == 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("SSL_CTX_set_cipher_list() failed");

				goto error;
			}

			// Enable ECDH ciphers.
			// DOC: http://en.wikibooks.org/wiki/OpenSSL/Diffie-Hellman_parameters
			// NOTE: https://code.google.com/p/chromium/issues/detail?id=406458
			// NOTE: https://bugs.ruby-lang.org/issues/12324

			// For OpenSSL >= 1.0.2.
			SSL_CTX_set_ecdh_auto(ssl_ctx_, 1);

			// Set the "use_srtp" DTLS extension.
			for (auto it = libsrtp::kSrtpCryptoSuites.begin();
				it != libsrtp::kSrtpCryptoSuites.end();
				++it)
			{
				if (it != libsrtp::kSrtpCryptoSuites.begin())
				{
					dtlsSrtpCryptoSuites += ":";
				}

				libsrtp::SrtpCryptoSuiteMapEntry* cryptoSuiteEntry = std::addressof(*it);
				dtlsSrtpCryptoSuites += cryptoSuiteEntry->name;
			}
			LIBSSL_LOG_T_F(LS_INFO) << "setting SRTP cryptoSuites for DTLS: " << dtlsSrtpCryptoSuites.c_str();
			//MS_DEBUG_2TAGS(dtls, srtp, "setting SRTP cryptoSuites for DTLS: %s", dtlsSrtpCryptoSuites.c_str());

			// NOTE: This function returns 0 on success.
			ret = SSL_CTX_set_tlsext_use_srtp(ssl_ctx_, dtlsSrtpCryptoSuites.c_str());

			if (ret != 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) <<
					"SSL_CTX_set_tlsext_use_srtp() failed when entering  :" << dtlsSrtpCryptoSuites.c_str();
				LIBSSL_LOG_T_F(LS_ERROR) << ("SSL_CTX_set_tlsext_use_srtp() failed");

				goto error;
			}

			return;

		error:

			if (ssl_ctx_)
			{
				SSL_CTX_free(ssl_ctx_);
				ssl_ctx_ = nullptr;
			}

			LIBSSL_LOG_T_F(LS_ERROR) << ("SSL context creation failed");
		}
		void DtlsCerts::GenerateFingerprints()
		{
			for (auto& kv : kString2FingerprintAlgorithm)
			{
				const std::string& algorithmString = kv.first;
				FingerprintAlgorithm algorithm = kv.second;
				uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
				unsigned int size{ 0 };
				char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
				const EVP_MD* hashFunction;
				int ret;

				switch (algorithm)
				{
				case FingerprintAlgorithm::SHA1:
					hashFunction = EVP_sha1();
					break;

				case FingerprintAlgorithm::SHA224:
					hashFunction = EVP_sha224();
					break;

				case FingerprintAlgorithm::SHA256:
					hashFunction = EVP_sha256();
					break;

				case FingerprintAlgorithm::SHA384:
					hashFunction = EVP_sha384();
					break;

				case FingerprintAlgorithm::SHA512:
					hashFunction = EVP_sha512();
					break;

				default:
				{
					LIBSSL_LOG_T_F(LS_ERROR) << ("unknown algorithm");
				}
				}

				ret = X509_digest(DtlsCerts::GetInstance().GetCertificate(), hashFunction, binaryFingerprint, &size);

				if (ret == 0)
				{
					LIBSSL_LOG_T_F(LS_ERROR) << ("X509_digest() failed");
					LIBSSL_LOG_T_F(LS_ERROR) << ("Fingerprints generation failed");
				}

				// Convert to hexadecimal format in uppercase with colons.
				for (unsigned int i{ 0 }; i < size; ++i)
				{
					std::sprintf(hexFingerprint + (i * 3), "%.2X:", binaryFingerprint[i]);
				}
				hexFingerprint[(size * 3) - 1] = '\0';

				//MS_DEBUG_TAG(dtls, "%-7s fingerprint: %s", algorithmString.c_str(), hexFingerprint);
				LIBSSL_LOG_T_F(LS_ERROR) << algorithmString << " fingerprint:" << hexFingerprint;
				// Store it in the vector.
				libssl::Fingerprint fingerprint;

				fingerprint.algorithm = DtlsCerts::GetFingerprintAlgorithm(algorithmString);
				fingerprint.value = hexFingerprint;

				local_fingerprints_.push_back(fingerprint);
			}
		}
		void DtlsCerts::GenerateCertificateAndPrivateKey()
		{
			int32_t ret = 0;
			EC_KEY* ecKey{ nullptr };
			X509_NAME* certName{ nullptr };
			std::string subject =
				std::string("librtc") + std::to_string(GenRandom());

			// Create key with curve.
			ecKey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

			if (!ecKey)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << "EC_KEY_new_by_curve_name() failed";
				//LOG_OPENSSL_ERROR("EC_KEY_new_by_curve_name() failed");

				goto error;
			}
			EC_KEY_set_asn1_flag(ecKey, OPENSSL_EC_NAMED_CURVE);

			// NOTE: This can take some time.
			ret = EC_KEY_generate_key(ecKey);

			if (ret == 0)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << "EC_KEY_generate_key() failed";

				goto error;
			}
			// Create a private key object.
			private_key_ = EVP_PKEY_new();

			if (!private_key_)
			{
				//LOG_OPENSSL_ERROR("EVP_PKEY_new() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "EVP_PKEY_new() failed";
				goto error;
			}

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
			ret = EVP_PKEY_assign_EC_KEY(private_key_, ecKey);

			if (ret == 0)
			{
				//LOG_OPENSSL_ERROR("EVP_PKEY_assign_EC_KEY() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "EVP_PKEY_assign_EC_KEY() failed";
				goto error;
			}
			// The EC key now belongs to the private key, so don't clean it up separately.
			ecKey = nullptr;

			// Create the X509 certificate.
			certificate_ = X509_new();

			if (!certificate_)
			{
				//LOG_OPENSSL_ERROR("X509_new() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "X509_new() failed";
				goto error;
			}

			// Set version 3 (note that 0 means version 1).
			X509_set_version(certificate_, 2);

			// Set serial number (avoid default 0).
			ASN1_INTEGER_set(
				X509_get_serialNumber(certificate_),
				static_cast<uint64_t>(GenRandom()));

			// Set valid period.
			X509_gmtime_adj(X509_get_notBefore(certificate_), -315360000); // -10 years.
			X509_gmtime_adj(X509_get_notAfter(certificate_), 315360000);   // 10 years.
			// Set the public key for the certificate using the key.
			ret = X509_set_pubkey(certificate_, private_key_);

			if (ret == 0)
			{
				//LOG_OPENSSL_ERROR("X509_set_pubkey() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "X509_set_pubkey() failed";
				goto error;
			}
			// Set certificate fields.
			certName = X509_get_subject_name(certificate_);

			if (!certName)
			{
				//LOG_OPENSSL_ERROR("X509_get_subject_name() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "X509_get_subject_name() failed";
				goto error;
			}

			X509_NAME_add_entry_by_txt(
				certName, "O", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);
			X509_NAME_add_entry_by_txt(
				certName, "CN", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);

			// It is self-signed so set the issuer name to be the same as the subject.
			ret = X509_set_issuer_name(certificate_, certName);

			if (ret == 0)
			{
				//LOG_OPENSSL_ERROR("X509_set_issuer_name() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "X509_set_issuer_name() failed";
				goto error;
			}
			// Sign the certificate with its own private key.
			ret = X509_sign(certificate_, private_key_, EVP_sha1());

			if (ret == 0)
			{
				//LOG_OPENSSL_ERROR("X509_sign() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "X509_sign() failed";
				goto error;
			}

			return;

		error:

			if (ecKey)
			{
				EC_KEY_free(ecKey);
			}

			if (private_key_)
			{
				EVP_PKEY_free(private_key_); // NOTE: This also frees the EC key.
			}

			if (certificate_)
			{
				X509_free(certificate_);
			}

			LIBSSL_LOG_T_F(LS_ERROR) << "DTLS certificate and private key generation failed";
		}
		void DtlsCerts::ReadCertificateAndPrivateKeyFromFiles(const char * dtls_certificate_file ,
			const char * dtls_private_key_file )
		{
			FILE* file{ nullptr };

			file = ::fopen(dtls_certificate_file, "r");

			if (!file)
			{
				///MS_ERROR("error reading DTLS certificate file: %s", std::strerror(errno));
				LIBSSL_LOG_T_F(LS_ERROR) << "error reading DTLS certificate file: %s" << std::strerror(errno);
				goto error;
			}

			certificate_ = PEM_read_X509(file, nullptr, nullptr, nullptr);

			if (!certificate_)
			{
				//LOG_OPENSSL_ERROR("PEM_read_X509() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "PEM_read_X509() failed" ;
				goto error;
			}

			fclose(file);

			file = fopen(dtls_private_key_file, "r");

			if (!file)
			{
				//MS_ERROR("error reading DTLS private key file: %s", std::strerror(errno));
				LIBSSL_LOG_T_F(LS_ERROR) << "error reading DTLS private key file: %s" << std::strerror(errno);
				goto error;
			}

			private_key_ = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);

			if (!private_key_)
			{
				//LOG_OPENSSL_ERROR("PEM_read_PrivateKey() failed");
				LIBSSL_LOG_T_F(LS_ERROR) << "PEM_read_PrivateKey() failed";// << std::strerror(errno);
				goto error;
			}

			fclose(file);

			return;

		error:

			LIBSSL_LOG_T_F(LS_ERROR) << "error reading DTLS certificate and private key PEM files";
		}
	}
}
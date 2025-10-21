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


#ifndef _C_DTLS_CERTS_H_
#define _C_DTLS_CERTS_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <map>
#include <vector>
#include <string>
#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "rtc_base/task_queue.h"
#include "libmedia_transfer_protocol/librtc/srtp_session.h"
#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>

#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/buffer.h"
namespace libmedia_transfer_protocol {
	namespace libssl {
		
	//	static const int32_t  
		
		//////////////////////////
		enum  class  FingerprintAlgorithm
		{
			NONE = 0,
			SHA1 = 1,
			SHA224,
			SHA256,
			SHA384,
			SHA512
		};
		struct Fingerprint
		{
			FingerprintAlgorithm algorithm{ FingerprintAlgorithm::NONE };
			std::string value;
		};
		// clang-format off
		//extern   std::map<  std::string, FingerprintAlgorithm> kString2FingerprintAlgorithm;
		//extern   std::map<FingerprintAlgorithm,    std::string> kFingerprintAlgorithm2String;
		class DtlsCerts
		{
		public:
			DtlsCerts();
			virtual ~DtlsCerts();


			static DtlsCerts &GetInstance()
			{
				static DtlsCerts  instanace;
				return instanace;
			}
		public:
			static FingerprintAlgorithm GetFingerprintAlgorithm(const std::string& fingerprint);
			
			static std::string  GetFingerprintAlgorithmString(FingerprintAlgorithm fingerprint);
			
			static bool IsDtls(const uint8_t* data, size_t len)
			{
				// clang-format off
				return (
					// Minimum DTLS record length is 13 bytes.
					(len >= 13) &&
					// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
					(data[0] > 19 && data[0] < 64)
					);
				// clang-format on
			}

		public:
			bool Init(const char * dtls_certificate_file = nullptr,
				const char * dtls_private_key_file = nullptr);
			void Destroy();
			 std::vector<libssl::Fingerprint> Fingerprints();
			EVP_PKEY *GetPrivateKey()const;
			  
			X509 * GetCertificate() const ;
			SSL_CTX * GetSslCtx() const;
		public:
			
			void GenerateCertificateAndPrivateKey();
			void ReadCertificateAndPrivateKeyFromFiles(const char * dtls_certificate_file = nullptr,
				const char * dtls_private_key_file = nullptr);
			
			void CreateSslCtx();
			void GenerateFingerprints();

			
			uint32_t GenRandom();
		private:
 
			////////////////////////////////////////////


			X509*      certificate_{ nullptr };
			EVP_PKEY*  private_key_{ nullptr };
			SSL_CTX*   ssl_ctx_{ nullptr };
			

			std::vector<libssl::Fingerprint> local_fingerprints_;
		};
	}


}


#endif // 
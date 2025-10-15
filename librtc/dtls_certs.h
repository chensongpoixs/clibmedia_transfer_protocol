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

namespace libmedia_transfer_protocol {
	namespace librtc {
		class DtlsCerts
		{
		public:
			DtlsCerts();
			virtual ~DtlsCerts();
		public:
			bool Init();
			const std::string &Fingerprint()const;
			EVP_PKEY *GetPrivateKey()const;
			X509 *GetCerts()const;
			uint32_t GenRandom();
		private:
			EVP_PKEY * dtls_pkey_{ nullptr };
			X509 * dtls_certs_{ nullptr };
			std::string fingerprint_;
		};
	}


}


#endif // 
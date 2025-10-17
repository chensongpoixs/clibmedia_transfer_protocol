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


#ifndef _C_LIBRTC_DTLS__H_
#define _C_LIBRTC_DTLS__H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
namespace libmedia_transfer_protocol {

	namespace librtc
	{
		class Dtls : public sigslot::has_slots<>
		{
		public:
			Dtls(/*DtlsHandler *handler*/) =default;
			~Dtls();

			bool Init();
			void OnRecv(const char *data, uint32_t size);
			const std::string &Fingerprint() const;
			void SetDone();
			void SetClient(bool client);
			const std::string &SendKey();
			const std::string &RecvKey();


			bool CheckStatus(int returnCode);

		public:

			sigslot::signal3<const char *, size_t, Dtls*>
				SignalDtlsSendPakcet;
			sigslot::signal1<  Dtls*>
				SignalDtlsHandshakeDone;
			// alter 
			sigslot::signal1<Dtls*>     SignalDtlsClose;
		private:
			bool InitSSLContext();
			bool InitSSL();
			static int SSLVerify(int preverify_ok, X509_STORE_CTX *ctx);
			static void SSLInfo(const SSL* ssl, int where, int ret);
			void NeedPost();
			void GetSrtpKey();
		private:

			SSL_CTX * ssl_context_{ nullptr };
			DtlsCerts dtls_cert_;
			bool is_client_{ false };
			bool is_done_{ false };
			bool handshake_done_{ false };
			SSL * ssl_{ nullptr };
			BIO * bio_read_{ nullptr };
			BIO * bio_write_{ nullptr };
			char buffer_[65535];
			//DtlsHandler * handler_{ nullptr };
			std::string send_key_;
			std::string recv_key_;
		};
	}

}

#endif //
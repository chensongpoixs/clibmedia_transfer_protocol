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
				   date:  2025-10-16



 ******************************************************************************/


#ifndef _C_LIBRTC_SRTP__H_
#define _C_LIBRTC_SRTP__H_

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
#include "rtc_base/buffer.h"
//#include "srtp.h"
//#include "usrsctp.h"


struct srtp_event_data_t;
struct srtp_ctx_t_;

namespace libmedia_transfer_protocol {

	namespace librtc
	{

		const int32_t kSrtpMaxBufferSize = 65535;
		class Srtp
		{
		public:
			Srtp() = default;
			~Srtp() = default;

			static bool InitSrtpLibrary();
			bool Init(const std::string &recv_key, const std::string &send_key);
			rtc::Buffer RtpProtect(rtc::Buffer &pkt);
			rtc::Buffer RtcpProtect(rtc::Buffer &pkt);
			rtc::Buffer SrtpUnprotect(rtc::Buffer &pkt);
			rtc::Buffer SrtcpUnprotect(rtc::Buffer &pkt);
			rtc::Buffer SrtcpUnprotect(const char *buf, size_t size);
		private:
			static void OnSrtpEvent(srtp_event_data_t* data);
		public:

			srtp_ctx_t_* send_ctx_{ nullptr };
			srtp_ctx_t_* recv_ctx_{ nullptr };
			char w_buffer_[kSrtpMaxBufferSize];
			char r_buffer_[kSrtpMaxBufferSize];
		};
	}
}

#endif // _C_LIBRTC_SRTP__H_
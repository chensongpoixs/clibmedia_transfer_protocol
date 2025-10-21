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


#ifndef _C_LIBRTC_SRTP_SESSION__H_
#define _C_LIBRTC_SRTP_SESSION__H_


#include "srtp.h"
//#include "usrsctp.h"

#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
struct srtp_event_data_t;
struct srtp_ctx_t_;

namespace libmedia_transfer_protocol {

	namespace libsrtp
	{
		////////////////////////////////
		enum  class  CryptoSuite
		{
			NONE = 0,
			AES_CM_128_HMAC_SHA1_80 = 1,
			AES_CM_128_HMAC_SHA1_32,
			AEAD_AES_256_GCM,
			AEAD_AES_128_GCM
		};
		enum   SrtpType
		{
			INBOUND = 1,
			OUTBOUND
		};
		// AES-HMAC: http://tools.ietf.org/html/rfc3711
		static const  size_t kSrtpMasterKeyLength{ 16 };
		static const  size_t kSrtpMasterSaltLength{ 14 };
		static const  size_t kSrtpMasterLength{ kSrtpMasterKeyLength + kSrtpMasterSaltLength };
		// AES-GCM: http://tools.ietf.org/html/rfc7714
		static const size_t kSrtpAesGcm256MasterKeyLength{ 32 };
		static const size_t kSrtpAesGcm256MasterSaltLength{ 12 };
		static const size_t kSrtpAesGcm256MasterLength{ kSrtpAesGcm256MasterKeyLength + kSrtpAesGcm256MasterSaltLength };
		static const size_t kSrtpAesGcm128MasterKeyLength{ 16 };
		static const size_t kSrtpAesGcm128MasterSaltLength{ 12 };
		static const size_t kSrtpAesGcm128MasterLength{ kSrtpAesGcm128MasterKeyLength + kSrtpAesGcm128MasterSaltLength };
		// clang-format on
		struct SrtpCryptoSuiteMapEntry
		{
			CryptoSuite   crypto_suite;
			const char *  name;
		};
		 extern   std::vector< SrtpCryptoSuiteMapEntry>   kSrtpCryptoSuites;
		//const int32_t kSrtpMaxBufferSize = 65535;
		static const size_t kEncryptBufferSize{ 65536 };
		class SrtpSession
		{
		public:
			SrtpSession(SrtpType type, CryptoSuite cryptoSuite, uint8_t* key, size_t keyLen)  ;
			~SrtpSession()  ;
		private:
			static void OnSrtpEvent(srtp_event_data_t* data);
		public:

			static bool InitSrtpLibrary();
			static void DestroySrtpLibrary();
			static const char* GetErrorString(srtp_err_status_t code);
		public:
			
			//bool Init(const std::string &recv_key, const std::string &send_key);
			//rtc::Buffer RtpProtect(rtc::Buffer &pkt);
			//rtc::Buffer RtcpProtect(rtc::Buffer &pkt);
			//rtc::Buffer SrtpUnprotect(rtc::Buffer &pkt);
			//rtc::Buffer SrtcpUnprotect(rtc::Buffer &pkt);
			//rtc::Buffer SrtcpUnprotect(const char *buf, size_t size);
			bool EncryptRtp(const uint8_t** data, size_t* len);
			bool DecryptSrtp(uint8_t* data, size_t* len);
			bool EncryptRtcp(const uint8_t** data, size_t* len);
			bool DecryptSrtcp(uint8_t* data, size_t* len);
			void RemoveStream(uint32_t ssrc);
			
		public:
			// Allocated by this.
			srtp_ctx_t_* session_{ nullptr };
			uint8_t EncryptBuffer[kEncryptBufferSize];
			//srtp_ctx_t_* send_ctx_{ nullptr };
			//srtp_ctx_t_* recv_ctx_{ nullptr };
			//char w_buffer_[kSrtpMaxBufferSize];
			//char r_buffer_[kSrtpMaxBufferSize];
		};
	}
}

#endif // _C_LIBRTC_SRTP_SESSION__H_
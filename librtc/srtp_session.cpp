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
#include "libmedia_transfer_protocol/librtc/srtp_session.h"
#include "rtc_base/logging.h"
#include "srtp.h"
#include "srtp.h"
#include "usrsctp.h"
//#include "rtc_base/location.h"
#include "srtp.h"
#include "srtp_priv.h"
#include "libmedia_transfer_protocol/librtc/rtc_errors.h"



namespace libmedia_transfer_protocol
{

	namespace libsrtp
	{
	    		std::vector< SrtpCryptoSuiteMapEntry>   kSrtpCryptoSuites = {
		{ CryptoSuite::AEAD_AES_256_GCM, "SRTP_AEAD_AES_256_GCM" },
		{ CryptoSuite::AEAD_AES_128_GCM, "SRTP_AEAD_AES_128_GCM" },
		{ CryptoSuite::AES_CM_128_HMAC_SHA1_80, "SRTP_AES128_CM_SHA1_80" },
		{ CryptoSuite::AES_CM_128_HMAC_SHA1_32, "SRTP_AES128_CM_SHA1_32" }
		};
		namespace
		{

			// clang-format off
			static      std::vector<const char*>  errors =
			{
				// From 0 (srtp_err_status_ok) to 24 (srtp_err_status_pfkey_err).
				"success (srtp_err_status_ok)",
				"unspecified failure (srtp_err_status_fail)",
				"unsupported parameter (srtp_err_status_bad_param)",
				"couldn't allocate memory (srtp_err_status_alloc_fail)",
				"couldn't deallocate memory (srtp_err_status_dealloc_fail)",
				"couldn't initialize (srtp_err_status_init_fail)",
				"can’t process as much data as requested (srtp_err_status_terminus)",
				"authentication failure (srtp_err_status_auth_fail)",
				"cipher failure (srtp_err_status_cipher_fail)",
				"replay check failed (bad index) (srtp_err_status_replay_fail)",
				"replay check failed (index too old) (srtp_err_status_replay_old)",
				"algorithm failed test routine (srtp_err_status_algo_fail)",
				"unsupported operation (srtp_err_status_no_such_op)",
				"no appropriate context found (srtp_err_status_no_ctx)",
				"unable to perform desired validation (srtp_err_status_cant_check)",
				"can’t use key any more (srtp_err_status_key_expired)",
				"error in use of socket (srtp_err_status_socket_err)",
				"error in use POSIX signals (srtp_err_status_signal_err)",
				"nonce check failed (srtp_err_status_nonce_bad)",
				"couldn’t read data (srtp_err_status_read_fail)",
				"couldn’t write data (srtp_err_status_write_fail)",
				"error parsing data (srtp_err_status_parse_err)",
				"error encoding data (srtp_err_status_encode_err)",
				"error while using semaphores (srtp_err_status_semaphore_err)",
				"error while using pfkey (srtp_err_status_pfkey_err)"
			};
			// clang-format on
			static rtc::Buffer null_packet(0);

		}
		bool SrtpSession::InitSrtpLibrary()
		{
			LIBRTC_LOG(LS_INFO) << "srtp library version:" << srtp_get_version();
			auto ret = srtp_init();
			if (ret != srtp_err_status_ok)
			{
				//MS_THROW_ERROR("srtp_install_event_handler() failed: %s", SrtpSession::GetErrorString(ret));
				throw std::runtime_error("srtp_install_event_handler() failed:");
				//LIBRTC_LOG_F(LS_WARNING) << "srtp init failed.";
				return false;
			}
			ret = srtp_install_event_handler(SrtpSession::OnSrtpEvent);
			return true;
		}
		void SrtpSession::DestroySrtpLibrary()
		{
			srtp_shutdown();
		}
		const char * SrtpSession::GetErrorString(srtp_err_status_t code)
		{
			return errors.at(code);
		}
		/*const char * SrtpSession::GetErrorString(srtp_err_status_t code)
		{
			return errors.at(code);
		}*/
	
		SrtpSession::SrtpSession(SrtpType type, CryptoSuite cryptoSuite, uint8_t * key, size_t keyLen)
		{
			srtp_policy_t policy; // NOLINT(cppcoreguidelines-pro-type-member-init)

		// Set all policy fields to 0.
			std::memset(&policy, 0, sizeof(srtp_policy_t));

			switch (cryptoSuite)
			{
			case CryptoSuite::AES_CM_128_HMAC_SHA1_80:
			{
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);

				break;
			}

			case CryptoSuite::AES_CM_128_HMAC_SHA1_32:
			{
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
				// NOTE: Must be 80 for RTCP.
				srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);

				break;
			}

			case CryptoSuite::AEAD_AES_256_GCM:
			{
				srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtp);
				srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtcp);

				break;
			}

			case CryptoSuite::AEAD_AES_128_GCM:
			{
				srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtp);
				srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtcp);

				break;
			}

			default:
			{
				RTC_CHECK_DESC(false, "unknown SRTP crypto suite");
			}
			}

			RTC_CHECK_DESC(
				(int)keyLen == policy.rtp.cipher_key_len,
				"given keyLen does not match policy.rtp.cipher_keyLen");

			switch (type)
			{
			case SrtpType::INBOUND:
				policy.ssrc.type = ssrc_any_inbound;
				break;

			case SrtpType::OUTBOUND:
				policy.ssrc.type = ssrc_any_outbound;
				break;
			}

			policy.ssrc.value = 0;
			policy.key = key;
			// Required for sending RTP retransmission without RTX.
			policy.allow_repeat_tx = 1;
			policy.window_size = 1024;
			policy.next = nullptr;

			// Set the SRTP session.
			srtp_err_status_t err = srtp_create(&this->session_, &policy);

			//if (DepLibSRTP::IsError(err))
				//MS_THROW_ERROR("srtp_create() failed: %s", DepLibSRTP::srtp_create() failed:(err));
			if (srtp_err_status_ok != err)
			{
				//MS_THROW_ERROR("srtp_create() failed:%s", SrtpSession::GetErrorString(err));
				//LIBSRTP_LOG_T_F(LS_ERROR) << "srtp_create() failed:" << SrtpSession:: GetErrorString(err);
				throw std::runtime_error("srtp_create() failed");
			}
		}
		SrtpSession::~SrtpSession()
		{
			if (this->session_ != nullptr)
			{
				srtp_err_status_t err = srtp_dealloc(this->session_);

				if (srtp_err_status_ok != err)
				{
					LIBSRTP_LOG_T_F(LS_ERROR) << "srtp_dealloc() failed:" << SrtpSession::GetErrorString(err);
				}
					//MS_ABORT("srtp_dealloc() failed: %s", DepLibSRTP::GetErrorString(err));
			}
		}
		void SrtpSession::OnSrtpEvent(srtp_event_data_t* data)
		{
			switch (data->event)
			{
			case event_ssrc_collision:
				//MS_WARN_TAG(srtp, "SSRC collision occurred");
				LIBSRTP_LOG_F(LS_WARNING) << "SSRC collision occurred";
				break;

			case event_key_soft_limit:
				LIBSRTP_LOG_F(LS_WARNING) << "stream reached the soft key usage limit and will expire soon";
				break;

			case event_key_hard_limit:
				LIBSRTP_LOG_F(LS_WARNING) << "stream reached the hard key usage limit and has expired";
				break;

			case event_packet_index_limit:
				LIBSRTP_LOG_F(LS_WARNING) << "stream reached the hard packet limit (2^48 packets)";
				break;
			}
		}
		bool SrtpSession::EncryptRtp(const uint8_t** data, size_t* len)
		{
			//MS_TRACE();

			// Ensure that the resulting SRTP packet fits into the encrypt buffer.
			if (*len + SRTP_MAX_TRAILER_LEN > kEncryptBufferSize)
			{
				LIBSRTP_LOG_T_F(LS_WARNING) << "cannot encrypt RTP packet, size too big ("<< *len <<" bytes)";

				return false;
			}

			std::memcpy(EncryptBuffer, *data, *len);

			srtp_err_status_t err =
				srtp_protect(this->session_, static_cast<void*>(EncryptBuffer), reinterpret_cast<int*>(len));

			if (srtp_err_status_ok != err)
			{
				LIBSRTP_LOG_T_F(LS_WARNING) << "srtp_protect() failed: " <<   GetErrorString(err);

				return false;
			}

			// Update the given data pointer.
			*data = (const uint8_t*)EncryptBuffer;

			return true;
		}

		bool SrtpSession::DecryptSrtp(uint8_t* data, size_t* len)
		{
			//MS_TRACE();

			srtp_err_status_t err =
				srtp_unprotect(this->session_, static_cast<void*>(data), reinterpret_cast<int*>(len));

			if (srtp_err_status_ok != err)
			{
				//MS_DEBUG_TAG(srtp, "srtp_unprotect() failed: %s", DepLibSRTP::GetErrorString(err));
				LIBSRTP_LOG_T_F(LS_INFO) << "srtp_unprotect() failed: " << GetErrorString(err);

				return false;
			}

			return true;
		}

		bool SrtpSession::EncryptRtcp(const uint8_t** data, size_t* len)
		{
			//MS_TRACE();

			// Ensure that the resulting SRTCP packet fits into the encrypt buffer.
			if (*len + SRTP_MAX_TRAILER_LEN > kEncryptBufferSize)
			{
				//MS_WARN_TAG(srtp, "cannot encrypt RTCP packet, size too big (%zu bytes)", *len);
				LIBSRTP_LOG_T_F(LS_WARNING) << "cannot encrypt RTCP packet, size too big (" << *len << " bytes)";

				return false;
			}

			std::memcpy(EncryptBuffer, *data, *len);

			srtp_err_status_t err = srtp_protect_rtcp(
				this->session_, static_cast<void*>(EncryptBuffer), reinterpret_cast<int*>(len));

			if (srtp_err_status_ok != err)
			{
				//MS_WARN_TAG(srtp, "srtp_protect_rtcp() failed: %s", DepLibSRTP::GetErrorString(err));
				LIBSRTP_LOG_T_F(LS_WARNING) << "srtp_protect_rtcp() failed: " << GetErrorString(err);

				return false;
			}

			// Update the given data pointer.
			*data = (const uint8_t*)EncryptBuffer;

			return true;
		}

		bool SrtpSession::DecryptSrtcp(uint8_t* data, size_t* len)
		{
			//MS_TRACE();

			srtp_err_status_t err =
				srtp_unprotect_rtcp(this->session_, static_cast<void*>(data), reinterpret_cast<int*>(len));

			if (srtp_err_status_ok != err)
			{
				//MS_DEBUG_TAG(srtp, "srtp_unprotect_rtcp() failed: %s", DepLibSRTP::GetErrorString(err));
				LIBSRTP_LOG_T_F(LS_INFO) << "srtp_unprotect_rtcp() failed: " << GetErrorString(err);

				return false;
			}

			return true;
		}

		void SrtpSession::RemoveStream(uint32_t ssrc)
		{
			srtp_remove_stream(this->session_, uint32_t{ htonl(ssrc) });
		}


	}
}
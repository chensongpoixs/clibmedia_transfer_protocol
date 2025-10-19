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
#include "srtp.h"
#include "srtp_priv.h"
namespace libmedia_transfer_protocol
{

	namespace librtc
	{
		namespace
		{
			static rtc::Buffer null_packet(0);
		}
		bool SrtpSession::InitSrtpLibrary()
		{
			LIBRTC_LOG(LS_INFO) << "srtp library version:" << srtp_get_version();
			auto ret = srtp_init();
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG_F(LS_WARNING) << "srtp init failed.";
				return false;
			}
			ret = srtp_install_event_handler(SrtpSession::OnSrtpEvent);
			return true;
		}
		bool SrtpSession::Init(const std::string &recv_key, const std::string &send_key)
		{
			srtp_policy_t srtp_policy;
			memset(&srtp_policy, 0, sizeof(srtp_policy_t));

			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_policy.rtp);
			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_policy.rtcp);

			srtp_policy.ssrc.value = 0;
			srtp_policy.window_size = 4096;
			srtp_policy.next = NULL;
			srtp_policy.allow_repeat_tx = 1;

			srtp_policy.ssrc.type = ssrc_any_inbound;
			srtp_policy.key = (unsigned char*)recv_key.c_str();
			auto ret = srtp_create(&recv_ctx_, &srtp_policy);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_create recv ctx failed.err=" << ret;
				return false;
			}

			srtp_policy.ssrc.type = ssrc_any_outbound;
			srtp_policy.key = (unsigned char*)send_key.c_str();
			ret = srtp_create(&send_ctx_, &srtp_policy);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_create send ctx failed.err=" << ret;
				return false;
			}
			return true;
		}
		rtc::Buffer SrtpSession::RtpProtect(rtc::Buffer &pkt)
		{
			int32_t bytes =  pkt.size();// pkt->PacketSize();

			if (bytes + SRTP_MAX_TRAILER_LEN >= kSrtpMaxBufferSize)
			{
				LIBRTC_LOG(LS_WARNING) << "pkt too large,bytes:" << bytes << " max:" << kSrtpMaxBufferSize;
				return rtc::Buffer(0);// null_packet;
			}

			std::memcpy(w_buffer_, pkt.begin(), bytes);
			auto ret = srtp_protect(send_ctx_, w_buffer_, &bytes);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_protect failed.err:" << ret;
				return rtc::Buffer(0);// null_packet;
			}

			rtc::Buffer npkt(bytes);// = Packet::NewPacket(bytes);
			std::memcpy(npkt.begin(), w_buffer_, bytes);
			npkt.SetSize(bytes);
			return npkt;
		}
		rtc::Buffer SrtpSession::RtcpProtect(rtc::Buffer &pkt)
		{
			int32_t bytes = pkt.size();

			if (bytes + SRTP_MAX_TRAILER_LEN >= kSrtpMaxBufferSize)
			{
				LIBRTC_LOG(LS_WARNING) << "pkt too large,bytes:" << bytes << " max:" << kSrtpMaxBufferSize;
				return rtc::Buffer(0);
			}

			std::memcpy(w_buffer_, pkt.begin(), bytes);
			auto ret = srtp_protect_rtcp(send_ctx_, w_buffer_, &bytes);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_protect_rtcp failed.err:" << ret;
				return rtc::Buffer(0);
			}

			rtc::Buffer npkt;// = Packet::NewPacket(bytes);
			std::memcpy(npkt.begin(), w_buffer_, bytes);
			npkt.SetSize(bytes);
			return npkt;
		}
		rtc::Buffer SrtpSession::SrtpUnprotect(rtc::Buffer &pkt)
		{
			int32_t bytes = pkt.size();

			if (bytes >= kSrtpMaxBufferSize)
			{
				LIBRTC_LOG(LS_WARNING) << "pkt too large,bytes:" << bytes << " max:" << kSrtpMaxBufferSize;
				return rtc::Buffer(0);
			}

			std::memcpy(r_buffer_, pkt.begin(), bytes);
			auto ret = srtp_unprotect(recv_ctx_, r_buffer_, &bytes);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_unprotect failed.err:" << ret;
				return rtc::Buffer(0);
			}

			rtc::Buffer npkt;// = Packet::NewPacket(bytes);
			std::memcpy(npkt.begin(), r_buffer_, bytes);
			npkt.SetSize(bytes);
			return npkt;
		}
		rtc::Buffer SrtpSession::SrtcpUnprotect(const char *buf, size_t size)
		{
			int32_t bytes = size;

			if (bytes >= kSrtpMaxBufferSize)
			{
				LIBRTC_LOG(LS_WARNING) << "pkt too large,bytes:" << bytes << " max:" << kSrtpMaxBufferSize;
				return rtc::Buffer(0);
			}

			std::memcpy(r_buffer_, buf, size);
			auto ret = srtp_unprotect_rtcp(recv_ctx_, r_buffer_, &bytes);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_unprotect_rtcp failed.err:" << ret;
				return rtc::Buffer(0);
			}

			rtc::Buffer npkt(bytes);// = Packet::NewPacket(bytes);
			std::memcpy(npkt.begin(), r_buffer_, bytes);
			npkt.SetSize(bytes);
			return npkt;
		}
		rtc::Buffer SrtpSession::SrtcpUnprotect(rtc::Buffer &pkt)
		{
			int32_t bytes = pkt.size();

			if (bytes >= kSrtpMaxBufferSize)
			{
				LIBRTC_LOG(LS_WARNING) << "pkt too large,bytes:" << bytes << " max:" << kSrtpMaxBufferSize;
				return rtc::Buffer(0);
			}

			std::memcpy(r_buffer_, pkt.begin(), bytes);
			auto ret = srtp_unprotect_rtcp(recv_ctx_, r_buffer_, &bytes);
			if (ret != srtp_err_status_ok)
			{
				LIBRTC_LOG(LS_WARNING) << "srtp_unprotect_rtcp failed.err:" << ret;
				return rtc::Buffer(0);
			}

			rtc::Buffer npkt(bytes);// = Packet::NewPacket(bytes);
			std::memcpy(npkt.begin(), r_buffer_, bytes);
			npkt.SetSize(bytes);
			return npkt;
		}
		void SrtpSession::OnSrtpEvent(srtp_event_data_t* data)
		{

		}
	}
}
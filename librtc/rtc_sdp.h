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


		1. 会话描述：会话版本，会话名称，会话时长，会话发起者
		2. 媒体信息描述：音视频格式列表，支持的协议列表，参数说明等
		3. 网络连接描述：网络类型，地址类型，IP地址
		4. 安全描述：ice用户名和密码，fingerprint
		5. 服务质量描述：RTCP反馈机制


 ******************************************************************************/


#ifndef _C_RTC_SDP_H_
#define _C_RTC_SDP_H_

#include <cstddef>

#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"

namespace libmedia_transfer_protocol {
	namespace librtc {
		class RtcSdp
		{
		public:
			RtcSdp();
			virtual ~RtcSdp();

		public:
			bool Decode(const std::string &sdp);
			const std::string &GetRemoteUFrag() const;
			const std::vector<libssl::Fingerprint> &GetLocalFingerprints()const;
			const libssl::Fingerprint  &  GetRemoteFingerprint() const;
			const std::string  &GetRemoteRole() const;
			int32_t GetVideoPayloadType() const;
			int32_t GetAudioPayloadType() const;

			void SetLocalFingerprint(const std::vector<libssl::Fingerprint> &fps);
			void SetStreamName(const std::string &name);
			void SetLocalUFrag(const std::string &frag);
			void SetLocalPasswd(const std::string &pwd);
			void SetServerPort(uint16_t port);
			void SetServerAddr(const std::string &addr);
			void SetVideoSsrc(uint32_t ssrc);
			void SetAudioSsrc(int32_t ssrc);
			const std::string &GetLocalPasswd()const;
			const std::string &GetLocalUFrag()const;
			uint32_t VideoSsrc() const;
			uint32_t AudioSsrc() const;
			std::string Encode();
		private:
			int32_t audio_payload_type_{ -1 };
			int32_t video_payload_type_{ -1 };
			// 远端的用户名和密码
			std::string remote_ufrag_;
			/*  a = setup 主要是表示dtls的协商过程中角色的问题，谁是客户端，谁是服务器
				a = setup:actpass 既可以是客户端，也可以是服务器
				a = setup : active 客户端
				a = setup : passive 服务器
				由客户端先发起client hello*/
			std::string remote_role_;// role = "active" / "passive" / "actpass" / "holdconn"
			std::string remote_passwd_;
			std::string local_ufrag_;
			std::string local_passwd_;
			//std::string remote_fingerprint_;
			libssl::Fingerprint  remote_fingerprint_;
			std::vector<libssl::Fingerprint>   finger_prints_;
			int32_t video_ssrc_{ 0 };
			int32_t audio_ssrc_{ 0 };
			int16_t server_port_{ 0 };
			std::string server_addr_;
			std::string stream_name_;
		};
	}
	

}


#endif // 
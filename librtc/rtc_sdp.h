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


		1. �Ự�������Ự�汾���Ự���ƣ��Ựʱ�����Ự������
		2. ý����Ϣ����������Ƶ��ʽ�б�֧�ֵ�Э���б�����˵����
		3. ���������������������ͣ���ַ���ͣ�IP��ַ
		4. ��ȫ������ice�û��������룬fingerprint
		5. ��������������RTCP��������


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
			// Զ�˵��û���������
			std::string remote_ufrag_;
			/*  a = setup ��Ҫ�Ǳ�ʾdtls��Э�̹����н�ɫ�����⣬˭�ǿͻ��ˣ�˭�Ƿ�����
				a = setup:actpass �ȿ����ǿͻ��ˣ�Ҳ�����Ƿ�����
				a = setup : active �ͻ���
				a = setup : passive ������
				�ɿͻ����ȷ���client hello*/
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
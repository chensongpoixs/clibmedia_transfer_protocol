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

single NAL Unit Mode:
当 packetization-mode 媒体类型参数的值等于 0，或者 packetization-mode 参数未出现时，使用此模式。所有接收端 必须 支持此模式。该模式主要用于与 ITU-T 推荐标准 H.241（见第 12.1 节）兼容的、低延迟应用场景。在此模式下，只能使用单一 NAL 单元包（Single NAL Unit Packets）。禁止使用 STAP（单时间聚合包）、MTAP（多时间聚合包）和 FU（分片单元）。发送时，单一 NAL 单元包的传输顺序必须遵循 NAL 单元的解码顺序。

non-interleaved mode:
当 packetization-mode（分包模式）这个可选的媒体类型参数的值为 1 时，使用此模式。此模式建议被支持。它主要面向低延迟应用场景。 在该模式下，允许使用以下类型的数据包：

● 单一 NAL 单元包（Single NAL Unit Packets）

● STAP-A（单时间聚合包 A）

● FU-A（分片单元 A）

而禁止使用以下类型的数据包：

● STAP-B（单时间聚合包 B）

● MTAP（多时间聚合包，包括 MTAP16 和 MTAP24）

● FU-B（分片单元 B）

同时，NAL 单元的发送顺序必须遵循 NAL 单元的解码顺序。

Interleaved Mode:
当 packetization-mode（分包模式）这个可选的媒体类型参数的值为 2 时，使用此模式。

部分接收端可以选择（MAY）支持此模式。

在该模式下，允许使用：

● STAP-B（单时间聚合包 B）

● MTAP（多时间聚合包，包括 MTAP16 和 MTAP24）

● FU-A（分片单元 A）

● FU-B（分片单元 B）

而禁止使用：

● STAP-A（单时间聚合包 A）

● 单一 NAL 单元包（Single NAL Unit Packets）
 

 ******************************************************************************/
#include "libmedia_transfer_protocol/librtc/rtc_sdp.h"
#include <vector>
#include "rtc_base/string_utils.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/string_utils.h"
#include <iostream>

namespace libmedia_transfer_protocol {
	namespace librtc
	{

		namespace {
			static const std::string rtpmap_token = "a=rtpmap:";
			static const std::string ice_ufrag_token = "a=ice-ufrag:";
			static const std::string ice_pwd_token = "a=ice-pwd:";
			static const std::string  fingerprint_token = "a=fingerprint:";
		}

		RtcSdp::RtcSdp() {}
		RtcSdp::  ~RtcSdp() {}


		bool RtcSdp::Decode(const std::string &sdp)
		{
			std::vector<std::string>    list;
			rtc::split(sdp, '\n', &list);
			if (list.size() < 0)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "parse sdp  failed !!! , line : " << list.size() << ", sdp : " << sdp;
				return false;
			}
			for (auto line : list)
			{
				//rtc::tokenize_first
				if (StringUtils::StartsWith(line, ice_ufrag_token))
				{
					remote_ufrag_ = line.substr(ice_pwd_token.size());
					LIBRTC_LOG(LS_INFO) << "remote ufrage:" << remote_ufrag_;
				}
				else if (StringUtils::StartsWith(line, ice_pwd_token))
				{
					remote_passwd_ = line.substr(ice_pwd_token.size());
					LIBRTC_LOG(LS_INFO) << "remote passwd:" << remote_passwd_;
				}
				else if (StringUtils::StartsWith(line, fingerprint_token))
				{
					fingerprint_ = line.substr(fingerprint_token.size());
					LIBRTC_LOG(LS_INFO) << "fingerprint:" << fingerprint_;
				}
				else if (StringUtils::StartsWith(line, rtpmap_token))
				{
					std::string content = line.substr(rtpmap_token.size());
					auto pos = content.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}

					int32_t pt = std::atoi(content.substr(0, pos).c_str());
					auto pos1 = content.find_first_of("/", pos + 1);
					if (pos1 == std::string::npos)
					{
						continue;
					}
					std::string name = content.substr(pos + 1, pos1 - pos - 1);
					if (audio_payload_type_ == -1 && name == "opus")
					{
						audio_payload_type_ = pt;
						LIBRTC_LOG(LS_INFO) << "audio_payload_type:" << audio_payload_type_;
					}
					else if (video_payload_type_ == -1 && name == "H264")
					{
						video_payload_type_ = pt;
						LIBRTC_LOG(LS_INFO) << "video_payload_type:" << video_payload_type_;
					}
				}
			}
		}
		const std::string &RtcSdp::GetRemoteUFrag() const
		{
			return remote_ufrag_;
		}
		const std::string &RtcSdp::GetFingerprint()const
		{
			return fingerprint_;
		}
		int32_t RtcSdp::GetVideoPayloadType() const
		{
			return video_payload_type_;
		}
		int32_t RtcSdp::GetAudioPayloadType() const
		{
			return  audio_payload_type_;
		}

		void RtcSdp::SetFingerprint(const std::string &fp)
		{
			fingerprint_ = fp;
		}
		void RtcSdp::SetStreamName(const std::string &name)
		{
			//stream_name_ = name;
			// 
			std::vector<std::string>  list;
			rtc::split(name, '/', &list);
			if (list.size() == 3)
			{
				stream_name_ = list[2];
			}
			else
			{
				stream_name_ = name;
			}
		}
		void RtcSdp::SetLocalUFrag(const std::string &frag)
		{
			local_ufrag_ = frag;
		}
		void RtcSdp::SetLocalPasswd(const std::string &pwd)
		{
			local_passwd_ = pwd;
		}
		void RtcSdp::SetServerPort(uint16_t port)
		{
			server_port_ = port;
		}
		void RtcSdp::SetServerAddr(const std::string &addr)
		{
			server_addr_ = addr;
		}
		void RtcSdp::SetVideoSsrc(uint32_t ssrc)
		{
			video_ssrc_ = ssrc;
		}
		void RtcSdp::SetAudioSsrc(int32_t ssrc)
		{
			audio_ssrc_ = ssrc;
		}
		const std::string &RtcSdp::GetLocalPasswd()const
		{
			return local_passwd_;
		}
		const std::string &RtcSdp::GetLocalUFrag()const
		{
			return local_ufrag_;
		}
		uint32_t RtcSdp::VideoSsrc() const
		{
			return video_ssrc_;
		}
		uint32_t RtcSdp::AudioSsrc() const
		{
			return audio_ssrc_;
		}
		std::string RtcSdp::Encode()
		{
			std::ostringstream ss;

			ss << "v=0\n";
			ss << "o=rtc 11111111111360111 2 IN IP4 0.0.0.0\n";
			ss << "s=" << stream_name_ << "\n";
			ss << "c=IN IP4 0.0.0.0\n";
			ss << "t=0 0\n";
			ss << "a=group:BUNDLE 0 1\n";
			ss << "a=msid-semantic: WMS " << stream_name_ << "\n";

			if (video_payload_type_ != -1 && audio_payload_type_ != -1)
			{
				ss << "m=audio 9 UDP/TLS/RTP/SAVPF " << audio_payload_type_ << "\n";
				ss << "c=IN IP4 0.0.0.0\n";
				
				ss << "a=mid:0\n";
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";

				ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
				ss << "a=fingerprint:sha-256 " << fingerprint_ << "\n";
				ss << "a=setup:passive\n"; 
				
				ss << "a=sendonly\n";
				ss << "a=rtcp-mux\n";
				ss << "a=rtcp-rsize\n";
				ss << "a=rtpmap:" << audio_payload_type_ << " opus/48000/2\n";
				ss << "a=fmtp:" << audio_payload_type_ << " minptime=10;stereo=1;useinbandfec=1\n";
				ss << "a=rtcp-fb:" << audio_payload_type_ << " transport-cc\n";
				ss << "a=rtcp-fb:" << audio_payload_type_ << " nack\n";
				ss << "a=ssrc:" << audio_ssrc_ << " cname:" << stream_name_ << "\n";
				ss << "a=ssrc:" << audio_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_audio\n";
				ss << "a=ssrc:" << audio_ssrc_ << " mslabel:" << stream_name_ << "\n";
				ss << "a=ssrc:" << audio_ssrc_ << " label:" << stream_name_ << "_audio\n";
			}
			if (video_payload_type_ != -1)
			{
				//std::vector<int32_t>
#if 0
				ss << "m=video 9 UDP/TLS/RTP/SAVPF " << video_payload_type_ << "\n"; 
#else
				ss << "m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 35 36 37 38 103 104 107 108 109 114 115 116 117 118 39 40 41 42 43 44 45 46 47 48 119 120 121 122 49 50 51 52 123 124 125 53\n";

#endif //
				ss << "c=IN IP4 0.0.0.0\n";
				ss << "a=mid:1\n";
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";
				ss << "a=fingerprint:sha-256 " << fingerprint_ << "\n";
				ss << "a=setup:passive\n";
				ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
				ss << "a=sendonly\n";
				ss << "a=rtcp-mux\n";
				ss << "a=rtcp-rsize\n";
#if 0
				ss << "a=rtpmap:" << video_payload_type_ << " H264/90000\n";
				/*
				 a=fmtp:123 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=64001f​

				level-asymmetry-allowed=1指明通信双方使用的H264Level是否要保持一致，0必须一致，1可以不一致。

				packetization-mode指明经H264编码后的视频数据如何打包：0单包、1非交错包、2交错包。三种打包模式中，模式0和模式1用于低延迟的实时通信领域。

				模式0的含义是每个包就是一帧视频数据。

				模式1是可以将视频帧拆分成多个顺序的RTP包发送，接收端收到数据包后再按顺序将其还原。

				profile-level-id由三部分组成，即profile_idc、profile_iop以及level_idc，每个组成占8位，
				因此可以推测出profile_idc=64、profile_iop=00、level-idc=1f

				 a=rtpmap:114 red/90000，red是一种在webrtc中使用的FEC（引入前向纠错）算法，用于防止丢包；
				 a=fmtp:103 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
				*/
				// a=fmtp:41 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=f4001f
				ss << "a=fmtp:"<< video_payload_type_ <<" level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\n";
				
				ss << "a=rtcp-fb:" << video_payload_type_ << " ccm fir\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " goog-remb\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack pli\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " transport-cc\n";

#else 
				ss << "a=rtpmap:96 VP8/90000\n";
				ss <<"a=rtcp-fb:96 goog-remb\n";
				ss <<"a=rtcp-fb:96 transport-cc\n";
				ss <<"a=rtcp-fb:96 ccm fir\n";
				ss <<"a=rtcp-fb:96 nack\n";
				ss <<"a=rtcp-fb:96 nack pli\n";
				ss <<"a=rtpmap:97 rtx/90000\n";
				ss <<"a=fmtp:97 apt=96\n";
				ss <<"a=rtpmap:98 VP9/90000\n";
				ss <<"a=rtcp-fb:98 goog-remb\n";
				ss <<"a=rtcp-fb:98 transport-cc\n";
				ss <<"a=rtcp-fb:98 ccm fir\n";
				ss <<"a=rtcp-fb:98 nack\n";
				ss <<"a=rtcp-fb:98 nack pli\n";
				ss <<"a=fmtp:98 profile-id=0\n";
				ss <<"a=rtpmap:99 rtx/90000\n";
				ss <<"a=fmtp:99 apt=98\n";
				ss <<"a=rtpmap:100 VP9/90000\n";
				ss <<"a=rtcp-fb:100 goog-remb\n";
				ss <<"a=rtcp-fb:100 transport-cc\n";
				ss <<"a=rtcp-fb:100 ccm fir\n";
				ss <<"a=rtcp-fb:100 nack\n";
				ss <<"a=rtcp-fb:100 nack pli\n";
				ss <<"a=fmtp:100 profile-id=2\n";
				ss <<"a=rtpmap:101 rtx/90000\n";
				ss <<"a=fmtp:101 apt=100\n";
				ss <<"a=rtpmap:35 VP9/90000\n";
				ss <<"a=rtcp-fb:35 goog-remb\n";
				ss <<"a=rtcp-fb:35 transport-cc\n";
				ss <<"a=rtcp-fb:35 ccm fir\n";
				ss <<"a=rtcp-fb:35 nack\n";
				ss <<"a=rtcp-fb:35 nack pli\n";
				ss <<"a=fmtp:35 profile-id=1\n";
				ss <<"a=rtpmap:36 rtx/90000\n";
				ss <<"a=fmtp:36 apt=35\n";
				ss <<"a=rtpmap:37 VP9/90000\n";
				ss <<"a=rtcp-fb:37 goog-remb\n";
				ss <<"a=rtcp-fb:37 transport-cc\n";
				ss <<"a=rtcp-fb:37 ccm fir\n";
				ss <<"a=rtcp-fb:37 nack\n";
				ss <<"a=rtcp-fb:37 nack pli\n";
				ss <<"a=fmtp:37 profile-id=3\n";
				ss <<"a=rtpmap:38 rtx/90000\n";
				ss <<"a=fmtp:38 apt=37\n";













				ss << "a=rtpmap:103 H264/90000\n";
				ss << "a=rtcp-fb:103 goog-remb\n";
				ss << "a=rtcp-fb:103 transport-cc\n";
				ss << "a=rtcp-fb:103 ccm fir\n";
				ss << "a=rtcp-fb:103 nack\n";
				ss << "a=rtcp-fb:103 nack pli\n";
				ss << "a=fmtp:103 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\n";
				ss << "a=rtpmap:104 rtx/90000\n";
				ss << "a=fmtp:104 apt=103\n";
				ss << "a=rtpmap:107 H264/90000\n";
				ss << "a=rtcp-fb:107 goog-remb\n";
				ss << "a=rtcp-fb:107 transport-cc\n";
				ss << "a=rtcp-fb:107 ccm fir\n";
				ss << "a=rtcp-fb:107 nack\n";
				ss << "a=rtcp-fb:107 nack pli\n";
				ss << "a=fmtp:107 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f\n";
				ss << "a=rtpmap:108 rtx/90000\n";
				ss << "a=fmtp:108 apt=107\n";
				ss << "a=rtpmap:109 H264/90000\n";
				ss << "a=rtcp-fb:109 goog-remb\n";
				ss << "a=rtcp-fb:109 transport-cc\n";
				ss << "a=rtcp-fb:109 ccm fir\n";
				ss << "a=rtcp-fb:109 nack\n";
				ss << "a=rtcp-fb:109 nack pli\n";
				ss << "a=fmtp:109 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f\n";
				ss << "a=rtpmap:114 rtx/90000\n";
				ss << "a=fmtp:114 apt=109\n";
				ss << "a=rtpmap:115 H264/90000\n";
				ss << "a=rtcp-fb:115 goog-remb\n";
				ss << "a=rtcp-fb:115 transport-cc\n";
				ss << "a=rtcp-fb:115 ccm fir\n";
				ss << "a=rtcp-fb:115 nack\n";
				ss << "a=rtcp-fb:115 nack pli\n";
				ss << "a=fmtp:115 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f\n";
				ss << "a=rtpmap:116 rtx/90000\n";
				ss << "a=fmtp:116 apt=115\n";
				ss << "a=rtpmap:117 H264/90000\n";
				ss << "a=rtcp-fb:117 goog-remb\n";
				ss << "a=rtcp-fb:117 transport-cc\n";
				ss << "a=rtcp-fb:117 ccm fir\n";
				ss << "a=rtcp-fb:117 nack\n";
				ss << "a=rtcp-fb:117 nack pli\n";
				ss << "a=fmtp:117 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d001f\n";
				ss << "a=rtpmap:118 rtx/90000\n";
				ss << "a=fmtp:118 apt=117\n";
				ss << "a=rtpmap:39 H264/90000\n";
				ss << "a=rtcp-fb:39 goog-remb\n";
				ss << "a=rtcp-fb:39 transport-cc\n";
				ss << "a=rtcp-fb:39 ccm fir\n";
				ss << "a=rtcp-fb:39 nack\n";
				ss << "a=rtcp-fb:39 nack pli\n";
				ss << "a=fmtp:39 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=4d001f\n";
				ss << "a=rtpmap:40 rtx/90000\n";
				ss << "a=fmtp:40 apt=39\n";
				ss << "a=rtpmap:41 H264/90000\n";
				ss << "a=rtcp-fb:41 goog-remb\n";
				ss << "a=rtcp-fb:41 transport-cc\n";
				ss << "a=rtcp-fb:41 ccm fir\n";
				ss << "a=rtcp-fb:41 nack\n";
				ss << "a=rtcp-fb:41 nack pli\n";
				ss << "a=fmtp:41 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=f4001f\n";
				ss << "a=rtpmap:42 rtx/90000\n";
				ss << "a=fmtp:42 apt=41\n";
				ss << "a=rtpmap:43 H264/90000\n";
				ss << "a=rtcp-fb:43 goog-remb\n";
				ss << "a=rtcp-fb:43 transport-cc\n";
				ss << "a=rtcp-fb:43 ccm fir\n";
				ss << "a=rtcp-fb:43 nack\n";
				ss << "a=rtcp-fb:43 nack pli\n";
				ss << "a=fmtp:43 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=f4001f\n";
				ss << "a=rtpmap:44 rtx/90000\n";
				ss << "a=fmtp:44 apt=43\n";
				ss << "a=rtpmap:45 AV1/90000\n";
				ss << "a=rtcp-fb:45 goog-remb\n";
				ss << "a=rtcp-fb:45 transport-cc\n";
				ss << "a=rtcp-fb:45 ccm fir\n";
				ss << "a=rtcp-fb:45 nack\n";
				ss << "a=rtcp-fb:45 nack pli\n";
				ss << "a=fmtp:45 level-idx=5;profile=0;tier=0\n";
				ss << "a=rtpmap:46 rtx/90000\n";
				ss << "a=fmtp:46 apt=45\n";
				ss << "a=rtpmap:47 AV1/90000\n";
				ss << "a=rtcp-fb:47 goog-remb\n";
				ss << "a=rtcp-fb:47 transport-cc\n";
				ss << "a=rtcp-fb:47 ccm fir\n";
				ss << "a=rtcp-fb:47 nack\n";
				ss << "a=rtcp-fb:47 nack pli\n";
				ss << "a=fmtp:47 level-idx=5;profile=1;tier=0\n";
				ss << "a=rtpmap:48 rtx/90000\n";
				ss << "a=fmtp:48 apt=47\n";
				ss << "a=rtpmap:119 H264/90000\n";
				ss << "a=rtcp-fb:119 goog-remb\n";
				ss << "a=rtcp-fb:119 transport-cc\n";
				ss << "a=rtcp-fb:119 ccm fir\n";
				ss << "a=rtcp-fb:119 nack\n";
				ss << "a=rtcp-fb:119 nack pli\n";
				ss << "a=fmtp:119 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=64001f\n";
				ss << "a=rtpmap:120 rtx/90000\n";
				ss << "a=fmtp:120 apt=119\n";
				ss << "a=rtpmap:121 H264/90000\n";
				ss << "a=rtcp-fb:121 goog-remb\n";
				ss << "a=rtcp-fb:121 transport-cc\n";
				ss << "a=rtcp-fb:121 ccm fir\n";
				ss << "a=rtcp-fb:121 nack\n";
				ss << "a=rtcp-fb:121 nack pli\n";
				ss << "a=fmtp:121 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=64001f\n";
				ss << "a=rtpmap:122 rtx/90000\n";
				ss << "a=fmtp:122 apt=121\n";
				ss << "a=rtpmap:49 H265/90000\n";
				ss << "a=rtcp-fb:49 goog-remb\n";
				ss << "a=rtcp-fb:49 transport-cc\n";
				ss << "a=rtcp-fb:49 ccm fir\n";
				ss << "a=rtcp-fb:49 nack\n";
				ss << "a=rtcp-fb:49 nack pli\n";
				ss << "a=fmtp:49 level-id=180;profile-id=1;tier-flag=0;tx-mode=SRST\n";
				ss << "a=rtpmap:50 rtx/90000\n";
				ss << "a=fmtp:50 apt=49\n";
				ss << "a=rtpmap:51 H265/90000\n";
				ss << "a=rtcp-fb:51 goog-remb\n";
				ss << "a=rtcp-fb:51 transport-cc\n";
				ss << "a=rtcp-fb:51 ccm fir\n";
				ss << "a=rtcp-fb:51 nack\n";
				ss << "a=rtcp-fb:51 nack pli\n";
				ss << "a=fmtp:51 level-id=180;profile-id=2;tier-flag=0;tx-mode=SRST\n";
				ss << "a=rtpmap:52 rtx/90000\n";
				ss << "a=fmtp:52 apt=51\n";
				ss << "a=rtpmap:123 red/90000\n";
				ss << "a=rtpmap:124 rtx/90000\n";
				ss << "a=fmtp:124 apt=123\n";
				ss << "a=rtpmap:125 ulpfec/90000\n";
				ss << "a=rtpmap:53 flexfec-03/90000\n";
				ss << "a=rtcp-fb:53 goog-remb\n";
				ss << "a=rtcp-fb:53 transport-cc\n";
				ss << "a=fmtp:53 repair-window=10000000\n";

























#endif 


				ss << "a=ssrc:" << video_ssrc_ << " cname:" << stream_name_ << "\n";
				ss << "a=ssrc:" << video_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_video\n";
				ss << "a=ssrc:" << video_ssrc_ << " mslabel:" << stream_name_ << "\n";
				ss << "a=ssrc:" << video_ssrc_ << " label:" << stream_name_ << "_video\n";
			}

			return ss.str();
		}
	}
	
 
}
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
				ss << "m=video 9 UDP/TLS/RTP/SAVPF " << video_payload_type_ << "\n"; 
				ss << "c=IN IP4 0.0.0.0\n";
				ss << "a=mid:1\n";
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";
				ss << "a=fingerprint:sha-256 " << fingerprint_ << "\n";
				ss << "a=setup:passive\n";
				ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
				ss << "a=rtpmap:" << video_payload_type_ << " H264/90000\n";
				
				ss << "a=sendonly\n";
				ss << "a=rtcp-mux\n";
				ss << "a=rtcp-rsize\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " ccm fir\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " goog-remb\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack pli\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " transport-cc\n";
				ss << "a=ssrc:" << video_ssrc_ << " cname:" << stream_name_ << "\n";
				ss << "a=ssrc:" << video_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_video\n";
				ss << "a=ssrc:" << video_ssrc_ << " mslabel:" << stream_name_ << "\n";
				ss << "a=ssrc:" << video_ssrc_ << " label:" << stream_name_ << "_video\n";
			}

			return ss.str();
		}
	}
	
 
}
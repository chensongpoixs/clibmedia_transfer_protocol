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
				   date:  2025-10-08

GB28181使用RTP传输音视频，有两种方式：UDP、TCP。UDP和RTSP中的没有区别，但是TCP有区别。

		目前RTSP有两个版本1.0和2.0，1.0定义在RFC2326中，2.0定义在RFC7826。2.0是2016年由IETF发布的RTSP新标准，不过现在基本使用的都是RTSP1.0，就算有使用2.0的，也会兼容1.0。
		而GB28181则使用RFC4571中定义的RTP，这里面RTP over TCP方式和以往的不同。

		RFC2326中RTP over TCP的数据包是这样的：

| magic number | channel number | data length | data  |magic number -

magic number：   RTP数据标识符，"$" 一个字节
channel number： 信道数字 - 1个字节，用来指示信道
data length ：   数据长度 - 2个字节，用来指示插入数据长度
data ：          数据 - ，比如说RTP包，总长度与上面的数据长度相同
		而RFC4571中的RTP over TCP的数据包确是这样的：

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	---------------------------------------------------------------
   |             LENGTH            |  RTP or RTCP packet ...       |
	---------------------------------------------------------------
		RFC2326中用channel number标识消息类型，因为RTSP中信令和和音视频都是通过同一个TCP通道传输，所以必须通过channel number区分。而GB28181中信令和媒体数据是不同的传输通道，所以不用去区分。

		RFC4571标准格式：长度(2字节) + RTP头 + 数据

		RFC2326标准格式：$(1字节) + 通道号(1字节) + 长度(2字节) + RTP头 + 数据

 ******************************************************************************/

#ifndef _C_LIBHTTP_UTILS_H_
#define _C_LIBHTTP_UTILS_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 ////////////////////
#include "libcross_platform_collection_render/video_render/cvideo_render_factory.h"
#include "libcross_platform_collection_render/video_render/cvideo_render.h"
#include "libcross_platform_collection_render/track_capture/ctrack_capture.h"
#include "libmedia_transfer_protocol/rtp_packet_sink_interface.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
#include "libmedia_codec/video_codec_type.h"
#include "libmedia_codec/video_codecs/h264_decoder.h"
#include "libmedia_codec/video_codecs/nal_parse_factory.h"
#include "libmedia_transfer_protocol/rtp_stream_receiver_controller.h"
#include "libmedia_transfer_protocol/librtsp/rtsp_session.h"
#include "libmedia_transfer_protocol/video_receive_stream.h"
#include "libp2p_peerconnection/connection_context.h"
#include "libcross_platform_collection_render/audio_capture/audio_capture.h"
#include "libmedia_transfer_protocol/libgb28181/gb28181_session.h"
#include "libmedia_transfer_protocol/libhttp/http_type.h"
#include <string>
#include <unordered_map>
#include <iostream>
//#include <ctype.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <algorithm>
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		class HttpUtils
		{
		public:
			static HttpMethod ParseMethod(const std::string &method);
			static HttpStatusCode ParseStatusCode(int32_t code);
			static std::string ParseStatusMessage(int32_t code);
			static ContentType ParseContentType(const std::string &contentType);
			static const std::string &ContentTypeToString(ContentType contenttype);
			static const std::string &StatusCodeToString(int code);
			static ContentType GetContentType(const std::string &fileName);
			static std::string CharToHex(char c);
			static bool NeedUrlDecoding(const std::string &url);
			static std::string UrlDecode(const std::string &url);
			static std::string UrlEncode(const std::string &src);

			static std::string& ltrim(std::string &str)
			{
				auto p = std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>( ::isspace)));
				str.erase(str.begin(), p);
				return str;
			}

			static std::string& rtrim(std::string &str)
			{
				auto p = std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>( ::isspace)));
				str.erase(p.base(), str.end());
				return str;
			}

			static std::string& Trim(std::string &str)
			{
				ltrim(rtrim(str));
				return str;
			}
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
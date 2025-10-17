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

#ifndef _C_HTTP_REQUEST_H_
#define _C_HTTP_REQUEST_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/libhttp/http_utils.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		class HttpRequest// : public   sigslot::has_slots<>
		{
		public:
			explicit HttpRequest(bool is_request = true);
			const std::unordered_map<std::string, std::string> &Headers() const;
			void AddHeader(const std::string &field, const std::string &value);
			void RemoveHeader(const std::string &key);
			const std::string &GetHeader(const std::string &field) const;
			void AddHeader(std::string &&field, std::string &&value);
			std::string MakeHeaders();
			void SetQuery(const std::string &query);
			void SetQuery(std::string &&query);
			void SetParameter(const std::string &key, const std::string &value);
			void SetParameter(std::string &&key, std::string &&value);
			const std::string &GetParameter(const std::string &key) const;
			const std::string &Query() const;

			void SetMethod(const std::string &method);
			void SetMethod(HttpMethod method);
			HttpMethod Method() const;
			void SetVersion(Version v);
			void SetVersion(const std::string &version);
			Version GetVersion() const;
			void SetPath(const std::string &path);
			const std::string &Path() const;
			void SetStatusCode(int32_t code);
			uint32_t GetStatusCode() const;
			void SetBody(const std::string &body);
			void SetBody(std::string &&body);
			const std::string &Body() const;
			std::string AppendToBuffer();
			bool IsRequest() const;
			bool IsStream() const;
			bool IsChunked() const;
			void SetIsStream(bool s);
			void SetIsChunked(bool c);

			static std::shared_ptr< HttpRequest> NewHttp400Response();
			static std::shared_ptr< HttpRequest>  NewHttp404Response();
			static std::shared_ptr< HttpRequest>  NewHttpOptionsResponse();
		private:
			void AppendRequestFirstLine(std::stringstream &ss);
			void AppendResponseFirstLine(std::stringstream &ss);

			void ParseParameters();

			HttpMethod method_{ kInvalid };
			Version version_{ Version::kUnknown };
			std::string path_;
			std::string query_;
			std::unordered_map<std::string, std::string> headers_;
			std::unordered_map<std::string, std::string> parameters_;
			std::string body_;
			uint32_t code_{ 0 };
			bool is_request_{ true };
			bool is_stream_{ false };
			bool is_chunked_{ false };

		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
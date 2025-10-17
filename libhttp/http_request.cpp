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

 
#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/libhttp/http_request.h"
#include "rtc_base/string_encode.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		namespace
		{
			static std::string string_empty;
		}

		HttpRequest::HttpRequest(bool is_request)
			:is_request_(is_request)
		{

		}
		std::shared_ptr< HttpRequest> HttpRequest::NewHttp400Response()
		{
			auto res = std::make_shared<HttpRequest>(false);
			res->SetStatusCode(400);
			res->AddHeader("User-Agent", "WebServer");
			res->AddHeader("Access-Control-Allow-Origin", "*");
			res->AddHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
			res->AddHeader("Allow", "POST, GET, OPTIONS");
			res->AddHeader("Access-Control-Allow-Headers", "content-type");
			return res;
		}
		std::shared_ptr< HttpRequest> HttpRequest::NewHttp404Response()
		{
			auto res = std::make_shared<HttpRequest>(false);
			res->SetStatusCode(404);
			res->AddHeader("User-Agent", "WebServer");
			res->AddHeader("Access-Control-Allow-Origin", "*");
			res->AddHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
			res->AddHeader("Allow", "POST, GET, OPTIONS");
			res->AddHeader("Access-Control-Allow-Headers", "content-type");
			return res;
		}
		std::shared_ptr< HttpRequest> HttpRequest::NewHttpOptionsResponse()
		{
			auto res = std::make_shared<HttpRequest>(false);
			res->SetStatusCode(200);
			res->AddHeader("server", "WebServer");
			res->AddHeader("content-length", "0");
			res->AddHeader("content-type", "text/plain");
			res->AddHeader("Access-Control-Allow-Origin", "*");
			res->AddHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
			res->AddHeader("Allow", "POST, GET, OPTIONS");
			res->AddHeader("Access-Control-Allow-Headers", "content-type");
			return res;
		}

		const std::unordered_map<std::string, std::string> &HttpRequest::Headers() const
		{
			return headers_;
		}
		void HttpRequest::AddHeader(const std::string &field, const std::string &value)
		{
			std::string k = field;
			std::transform(k.begin(), k.end(), k.begin(), ::tolower);
			headers_[k] = value;
		}
		void HttpRequest::RemoveHeader(const std::string &key)
		{
			std::string k = key;
			std::transform(k.begin(), k.end(), k.begin(), ::tolower);
			headers_.erase(k);
		}
		const std::string &HttpRequest::GetHeader(const std::string &field) const
		{
			std::string k = field;
			std::transform(k.begin(), k.end(), k.begin(), ::tolower);
			auto iter = headers_.find(k);
			if (iter != headers_.end())
			{
				return iter->second;
			}
			return string_empty;
		}
		void HttpRequest::AddHeader(std::string &&field, std::string &&value)
		{
			std::transform(field.begin(), field.end(), field.begin(), ::tolower);
			headers_[std::move(field)] = std::move(value);
		}

		std::string HttpRequest::MakeHeaders()
		{
			std::stringstream ss;
			if (is_request_)
			{
				AppendRequestFirstLine(ss);
			}
			else
			{
				AppendResponseFirstLine(ss);
			}
			for (auto const &h : headers_)
			{
				ss << h.first << ": " << h.second << "\r\n";
			}
			// if(!body_.empty())
			// {
			//     ss << "content-length: " << body_.size() << "\r\n";
			// }
			// else 
			// {
			//     ss << "content-length: 0\r\n";
			// }
			ss << "\r\n";
			return ss.str();
		}

		void HttpRequest::SetQuery(const std::string &query)
		{
			query_ = query;
			ParseParameters();
		}
		void HttpRequest::SetQuery(std::string &&query)
		{
			query_ = std::move(query);
			ParseParameters();
		}
		void HttpRequest::SetParameter(const std::string &key, const std::string &value)
		{
			parameters_[key] = value;
		}
		void HttpRequest::SetParameter(std::string &&key, std::string &&value)
		{
			parameters_[std::move(key)] = std::move(value);
		}
		const std::string &HttpRequest::GetParameter(const std::string &key) const
		{
			auto iter = parameters_.find(key);
			if (iter != parameters_.end())
			{
				return iter->second;
			}
			return string_empty;
		}
		const std::string &HttpRequest::Query() const
		{
			return query_;
		}
		void HttpRequest::ParseParameters()
		{
			std::vector<std::string> list;
			rtc::split(query_, '&', &list);
			 
			for (auto const &l : list)
			{
				auto pos = l.find('=');
				if (pos != std::string::npos)
				{
					std::string k = l.substr(0, pos);
					std::string v = l.substr(pos + 1);
					k = HttpUtils::Trim(k);
					v = HttpUtils::Trim(v);
					SetParameter(std::move(k), std::move(v));
				}
			}
		}

		void HttpRequest::SetMethod(const std::string &method)
		{
			method_ = HttpUtils::ParseMethod(method);
		}
		void HttpRequest::SetMethod(HttpMethod method)
		{
			method_ = method;
		}
		HttpMethod HttpRequest::Method() const
		{
			return method_;
		}
		void HttpRequest::SetVersion(const std::string &v)
		{
			version_ = Version::kUnknown;
			if (v.size() == 8) // http/1.0
			{
				if (v.compare(0, 6, "HTTP/1."))
				{
					if (v[7] == '1')
					{
						version_ = Version::kHttp11;
					}
					else if (v[7] == '0')
					{
						version_ = Version::kHttp10;
					}
				}
			}
		}
		void HttpRequest::SetVersion(Version v)
		{
			version_ = v;
		}
		Version HttpRequest::GetVersion() const
		{
			return version_;
		}
		void HttpRequest::SetPath(const std::string &path)
		{
			if (HttpUtils::NeedUrlDecoding(path))
			{
				path_ = HttpUtils::UrlDecode(path);
			}
			else
			{
				path_ = path;
			}
		}
		const std::string &HttpRequest::Path() const
		{
			return path_;
		}

		void HttpRequest::SetStatusCode(int32_t code)
		{
			code_ = code;
		}
		uint32_t HttpRequest::GetStatusCode() const
		{
			return code_;
		}
		void HttpRequest::SetBody(std::string &&body)
		{
			body_ = std::move(body);
		}
		void HttpRequest::SetBody(const std::string &body)
		{
			body_ = body;
		}
		const std::string &HttpRequest::Body() const
		{
			return body_;
		}

		std::string HttpRequest::AppendToBuffer()
		{
			std::stringstream ss;

			ss << MakeHeaders();
			if (!body_.empty())
			{
				ss << body_;
			}
			return ss.str();
		}
		bool HttpRequest::IsRequest() const
		{
			return is_request_;
		}
		bool HttpRequest::IsStream() const
		{
			return is_stream_;
		}
		bool HttpRequest::IsChunked() const
		{
			return is_chunked_;
		}
		void HttpRequest::SetIsStream(bool s)
		{
			is_stream_ = s;
		}
		void HttpRequest::SetIsChunked(bool c)
		{
			is_chunked_ = c;
		}
		void HttpRequest::AppendRequestFirstLine(std::stringstream &ss)
		{
			switch (method_)
			{
			case kGet:
			{
				ss << "GET ";
				break;
			}
			case kPost:
			{
				ss << "POST ";
				break;
			}
			case kHead:
			{
				ss << "HEAD ";
				break;
			}
			case kPut:
			{
				ss << "PUT ";
				break;
			}
			case kDelete:
			{
				ss << "DELETE ";
				break;
			}
			case kOptions:
			{
				ss << "OPTIONS ";
				break;
			}
			case kPatch:
			{
				ss << "PATCH ";
				break;
			}
			default:
			{
				ss << "UNKNOW ";
				break;
			}
			}

			std::stringstream sss;
			if (!path_.empty())
			{
				sss << path_;
			}
			else
			{
				sss << "/";
			}
			if (!parameters_.empty())
			{
				sss << "?";
				for (auto iter = parameters_.begin(); iter != parameters_.end(); iter++)
				{
					if (iter == parameters_.begin())
					{
						sss << iter->first << "=" << iter->second;
					}
					else
					{
						sss << "&" << iter->first << "=" << iter->second;
					}
				}
			}
			ss << HttpUtils::UrlEncode(sss.str()) << " ";
			if (version_ == Version::kHttp10)
			{
				ss << "HTTP/1.0 ";
			}
			else
			{
				ss << "HTTP/1.1 ";
			}
			ss << "\r\n";
		}
		void HttpRequest::AppendResponseFirstLine(std::stringstream &ss)
		{
			if (version_ == Version::kHttp10)
			{
				ss << "HTTP/1.0 ";
			}
			else
			{
				ss << "HTTP/1.1 ";
			}

			ss << code_ << " ";
			ss << HttpUtils::ParseStatusMessage(code_);
			ss << "\r\n";
		}

	}

}
 
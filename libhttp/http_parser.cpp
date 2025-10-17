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
#include "libmedia_transfer_protocol/libhttp/http_parser.h"
#include <algorithm>

#include "rtc_base/logging.h"

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		
		namespace
		{
			static std::string string_empty;
			static std::string CRLFCRLF = "\r\n\r\n";
			static int32_t kHttpMaxBodySize = 64 * 1024;
			static constexpr char CRLF[]{ "\r\n" };
		}


		HttpParserState HttpParser::Parse(MsgBuffer &buf)
		{
			if (buf.ReadableBytes() == 0)
			{
				return state_;
			}
			if (state_ == kExpectHttpComplete)
			{
				ClearForNextHttp();
			}
			else if (state_ == kExpectChunkComplete)
			{
				ClearForNextChunk();
			}
			switch (state_)
			{
			case kExpectHeaders:
			{
				if (buf.ReadableBytes() > CRLFCRLF.size())
				{
					auto *space = std::search(buf.Peek(), (const char *)buf.BeginWrite(), CRLFCRLF.data(), CRLFCRLF.data() + CRLFCRLF.size());
					if (space != (const char *)buf.BeginWrite())
					{
						auto size = space - buf.Peek();
						header_.assign(buf.Peek(), size);
						buf.Retrieve(size + 4);
						ParseHeaders();
						if (state_ == kExpectHttpComplete || state_ == kExpectError)
						{
							return state_;
						}
					}
					else
					{
						if (buf.ReadableBytes() > kHttpMaxBodySize)
						{
							reason_ = k400BadRequest;
							state_ = kExpectError;
							return state_;
						}
						return kExpectContinue;
					}
				}
				else
				{
					return kExpectContinue;
				}
			}
			break;
			case kExpectNormalBody:
			{
				ParseNormalBody(buf);
				break;
			}
			case kExpectStreamBody:
			{
				ParseStream(buf);
				break;
			}
			case kExpectChunkLen:
			{
				auto crlf = buf.FindCRLF();
				if (crlf)
				{
					std::string len(buf.Peek(), crlf);
					char *end;
					current_chunk_length_ = std::strtol(len.c_str(), &end, 16);

                     // RTC_LOG(rtc::LS_INFO) << "";
					LIBHTTP_LOG(LS_INFO) << " chunk len:" << current_chunk_length_;

					if (current_chunk_length_ > 1024 * 1024)
					{
						LIBHTTP_LOG(LS_ERROR) << "error chunk len.";

						state_ = kExpectError;
						reason_ = k400BadRequest;
					}
					buf.RetrieveUntil(crlf + 2);
					if (current_chunk_length_ == 0)
					{
						state_ = kExpectLastEmptyChunk;
					}
					else
					{
						current_chunk_length_ += 2;
						state_ = kExpectChunkBody;
					}
				}
				else
				{
					if (buf.ReadableBytes() > 32)
					{
						buf.RetrieveAll();
						reason_ = k400BadRequest;
						state_ = kExpectError;
						return state_;
					}
				}
				break;
			}
			case kExpectChunkBody:
			{
				ParseChunk(buf);
				if (state_ == kExpectChunkComplete)
				{
					return state_;
				}
				break;
			}
			case kExpectLastEmptyChunk:
			{
				auto crlf = buf.FindCRLF();
				if (crlf)
				{
					buf.RetrieveUntil(crlf + 2);
					chunk_.reset();
					state_ = kExpectChunkComplete;
					break;
				}
			}
			default:
				break;
			}
			return state_;
		}
		void HttpParser::ParseStream(MsgBuffer &buf)
		{
			if (!chunk_)
			{
				chunk_ = Packet::NewPacket(kHttpMaxBodySize);
			}
			auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
			memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
			chunk_->UpdatePacketSize(size);
			buf.Retrieve(size);

			if (chunk_->Space() == 0)
			{
				state_ = kExpectChunkComplete;
			}
		}
		void HttpParser::ParseNormalBody(MsgBuffer &buf)
		{
			if (!chunk_)
			{
				chunk_ = Packet::NewPacket(current_content_length_);
			}
			auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
			memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
			chunk_->UpdatePacketSize(size);
			buf.Retrieve(size);
			current_content_length_ -= size;
			if (current_content_length_ == 0)
			{
				state_ = kExpectHttpComplete;
			}
		}
		void HttpParser::ParseChunk(MsgBuffer &buf)
		{
			if (!chunk_)
			{
				chunk_ = Packet::NewPacket(current_chunk_length_);
			}
			auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
			memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
			chunk_->UpdatePacketSize(size);
			buf.Retrieve(size);
			current_chunk_length_ -= size;
			if (current_chunk_length_ == 0 || chunk_->Space() == 0)
			{
				chunk_->SetPacketSize(chunk_->PacketSize() - 2);
				state_ = kExpectChunkComplete;
			}
		}
		void HttpParser::ParseHeaders()
		{
			//auto list = base::StringUtils::SplitString(header_, "\r\n");
			std::vector<std::string> list;
			rtc::split(header_, '\n', &list);
			if (list.size() < 1)
			{
				reason_ = k400BadRequest;
				state_ = kExpectError;
				return;
			}
			ProcessMethodLine(list[0]);

			for (auto &l : list)
			{
				auto pos = l.find_first_of(':');
				if (pos != std::string::npos)
				{
					std::string k = l.substr(0, pos);
					std::string v = l.substr(pos + 1);
					k = HttpUtils::Trim(k);
					v = HttpUtils::Trim(v);
					LIBHTTP_LOG(LS_INFO) << "parse header k:" << k << " v:" << v;
					req_->AddHeader(std::move(k), std::move(v));
				}
			}
			auto len = req_->GetHeader("content-length");
			if (!len.empty())
			{
				LIBHTTP_LOG(LS_INFO) << "content-length:" << len;
				try
				{
					current_content_length_ = std::stoull(len);
				}
				catch (...)
				{
					reason_ = k400BadRequest;
					state_ = kExpectError;
					return;
				}

				if (current_content_length_ == 0)
				{
					state_ = kExpectHttpComplete;
				}
				else
				{
					state_ = kExpectNormalBody;
				}
			}
			else
			{
				const std::string &chunk = req_->GetHeader("transfer-encoding");
				if (!chunk.empty() && chunk == "chunked")
				{
					is_chunked_ = true;
					req_->SetIsChunked(true);
					state_ = kExpectChunkLen;
				}
				else
				{
					if ((!is_request_&&req_->GetStatusCode() != 200)
						|| (is_request_
							&& (req_->Method() == kGet
								|| req_->Method() == kHead
								|| req_->Method() == kOptions)))
					{
						current_chunk_length_ = 0;
						state_ = kExpectHttpComplete;
					}
					else
					{
						current_content_length_ = -1;
						is_stream_ = true;
						req_->SetIsStream(true);
						state_ = kExpectStreamBody;
					}
				}
			}
		}
		void HttpParser::ProcessMethodLine(const std::string &line)
		{
			LIBHTTP_LOG(LS_INFO) << "parse method line:" << line;
			//auto list = base::StringUtils::SplitString(line, " ");
			std::vector<std::string> list;
			rtc::split(line, ' ', &list);
			std::string str = list[0];
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			if (str[0] == 'h'&&str[1] == 't'&& str[2] == 't'&&str[3] == 'p')
			{
				is_request_ = false;
			}
			else
			{
				is_request_ = true;
			}

			if (req_)
			{
				req_.reset();
			}
			req_ = std::make_shared<HttpRequest>(is_request_);
			if (is_request_)
			{
				req_->SetMethod(list[0]);
				const std::string &path = list[1];
				auto pos = path.find_first_of("?");
				if (pos != std::string::npos)
				{
					req_->SetPath(path.substr(0, pos));
					req_->SetQuery(path.substr(pos + 1));
				}
				else
				{
					req_->SetPath(path);
				}
				req_->SetVersion(list[2]);
				LIBHTTP_LOG(LS_INFO) << "http method:" << list[0]
					<< " path:" << req_->Path()
					<< " query:" << req_->Query()
					<< " version:" << list[2];
			}
			else
			{
				req_->SetVersion(list[0]);
				req_->SetStatusCode(std::atoi(list[1].c_str()));
				LIBHTTP_LOG(LS_INFO) << "http code:" << list[1]
					<< " version:" << list[0];
			}
		}

		const std::shared_ptr<Packet> &HttpParser::Chunk() const
		{
			return chunk_;
		}
		HttpStatusCode HttpParser::Reason() const
		{
			return reason_;
		}
		void HttpParser::ClearForNextHttp()
		{
			state_ = kExpectHeaders;
			header_.clear();
			req_.reset();
			current_content_length_ = -1;
			chunk_.reset();
		}
		void HttpParser::ClearForNextChunk()
		{
			if (is_chunked_)
			{
				state_ = kExpectChunkLen;
				current_chunk_length_ = -1;
			}
			else
			{
				if (is_stream_)
				{
					state_ = kExpectStreamBody;
				}
				else
				{
					state_ = kExpectHeaders;
					current_chunk_length_ = -1;
				}
			}
			chunk_.reset();
		}
	}

}

 
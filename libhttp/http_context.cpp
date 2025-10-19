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
 ////////////////////
#include "libmedia_transfer_protocol/libhttp/http_context.h"
#include "libmedia_transfer_protocol/libhttp/http_session.h"
#include "libmedia_transfer_protocol/libhttp/packet.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"
#include "libmedia_transfer_protocol/libhttp/http_parser.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		namespace
		{
			static std::string CHUNK_EOF = "0\r\n\r\n";
		}
		HttpContext::HttpContext(   TcpSession*conn/*, HttpHandler *handler*/)
			:connection_(conn)//, handler_(handler)
		{

		}

		int32_t HttpContext::Parse(MsgBuffer &buf)
		{
			while (buf.ReadableBytes() > 1)
			{
				auto state = http_parser_.Parse(buf);
				if (state == kExpectHttpComplete || state == kExpectChunkComplete)
				{
					SignalOnRequest(connection_, http_parser_.GetHttpRequest(), http_parser_.Chunk());
					 
				}
				else if (state == kExpectError)
				{

					connection_->Close();
				}
			}
			return 1;
		}

		bool HttpContext::PostRequest(const std::string &header_and_body)
		{
			if (post_state_ != kHttpContextPostInit)
			{
				return false;
			}
			header_ = header_and_body;
			post_state_ = kHttpContextPostHttp;
			connection_->Send((uint8_t*)header_.c_str(), header_.size());
			return true;
		}
		bool HttpContext::PostRequest(const std::string &header, std::shared_ptr<Packet> &packet)
		{
			if (post_state_ != kHttpContextPostInit)
			{
				return false;
			}

			header_ = header;
			out_pakcet_ = packet;
			post_state_ = kHttpContextPostHttpHeader;
			connection_->Send((uint8_t*)header_.c_str(), header_.size());
			return true;
		}
		bool HttpContext::PostRequest(std::shared_ptr< HttpRequest>  &request)
		{
			if (request->IsChunked())
			{
				PostChunkHeader(request->MakeHeaders());
			}
			else if (request->IsStream())
			{
				PostStreamHeader(request->MakeHeaders());
			}
			else
			{
				PostRequest(request->AppendToBuffer());
			}
			return true;
		}
		bool HttpContext::PostChunkHeader(const std::string &header)
		{
			if (post_state_ != kHttpContextPostInit)
			{
				return false;
			}

			header_ = header;
			post_state_ = kHttpContextPostInit;
			connection_->Send((uint8_t *)header_.c_str(), header_.size());
			header_sent_ = true;
			return true;
		}
		void HttpContext::PostChunk(std::shared_ptr< Packet>  &chunk)
		{
			out_pakcet_ = chunk;
			if (!header_sent_)
			{
				post_state_ = kHttpContextPostChunkHeader;
				connection_->Send((uint8_t *)header_.c_str(), header_.size());
				header_sent_ = true;
			}
			else
			{
				post_state_ = kHttpContextPostChunkLen;
				char buf[32] = { 0, };
				sprintf(buf, "%X\r\n", out_pakcet_->PacketSize());
				header_ = std::string(buf);
				connection_->Send((uint8_t *)header_.c_str(), header_.size());
			}
		}
		void HttpContext::PostEofChunk()
		{
			post_state_ = kHttpContextPostChunkEOF;
			connection_->Send((uint8_t *)CHUNK_EOF.c_str(), CHUNK_EOF.size());
		}
		bool HttpContext::PostStreamHeader(const std::string &header)
		{
			if (post_state_ != kHttpContextPostInit)
			{
				return false;
			}

			header_ = header;
			post_state_ = kHttpContextPostInit;
			connection_->Send((uint8_t *)header_.c_str(), header_.size());
			header_sent_ = true;
			return true;
		}
		bool HttpContext::PostStreamChunk(std::shared_ptr<Packet> &packet)
		{
			if (post_state_ == kHttpContextPostInit)
			{
				out_pakcet_ = packet;
				if (!header_sent_)
				{
					post_state_ = kHttpContextPostHttpStreamHeader;
					connection_->Send((uint8_t *)header_.c_str(), header_.size());
					header_sent_ = true;
				}
				else
				{
					post_state_ = kHttpContextPostHttpStreamChunk;
					connection_->Send((uint8_t *)out_pakcet_->Data(), out_pakcet_->PacketSize());
				}
				return true;
			}
			return false;
		}
		void HttpContext::WriteComplete(  TcpSession *conn)
		{
			switch (post_state_)
			{
				case kHttpContextPostInit:
				{
					break;
				}
				case kHttpContextPostHttp:
				{
					post_state_ = kHttpContextPostInit;
					SignalOnSent(conn);
					break;
				}
				case kHttpContextPostHttpHeader:
				{
					post_state_ = kHttpContextPostHttpBody;
					connection_->Send((uint8_t*)out_pakcet_->Data(), out_pakcet_->PacketSize());
					break;
				}
				case kHttpContextPostHttpBody:
				{
					post_state_ = kHttpContextPostInit;
					//handler_->OnSent(conn);
					SignalOnSent(conn);
					break;
				}
				case kHttpContextPostChunkHeader:
				{
					post_state_ = kHttpContextPostChunkLen;
					//handler_->OnSentNextChunk(conn);
					SignalOnSentNextChunk(conn);
					break;
				}
				case kHttpContextPostChunkLen:
				{
					post_state_ = kHttpContextPostChunkBody;
					connection_->Send((uint8_t*)out_pakcet_->Data(), out_pakcet_->PacketSize());
					break;
				}
				case kHttpContextPostChunkBody:
				{
					post_state_ = kHttpContextPostInit;
					//handler_->OnSentNextChunk(conn);
					SignalOnSentNextChunk(conn);
					break;
				}
				case kHttpContextPostChunkEOF:
				{
					post_state_ = kHttpContextPostInit;
					//handler_->OnSent(conn);
					SignalOnSent(conn);
					break;
				}
				case kHttpContextPostHttpStreamHeader:
				{
					post_state_ = kHttpContextPostInit;
					//handler_->OnSentNextChunk(conn);
					SignalOnSentNextChunk(conn);
					break;
				}
				case kHttpContextPostHttpStreamChunk:
				{
					post_state_ = kHttpContextPostInit;
					//handler_->OnSentNextChunk(conn);
					SignalOnSentNextChunk(conn);
					break;
				}
			}


		}
	}

}

 
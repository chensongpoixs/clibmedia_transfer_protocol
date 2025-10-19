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

#ifndef _C_LIBHTTP_SERVER_H_
#define _C_LIBHTTP_SERVER_H_

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
#include "libmedia_transfer_protocol/libhttp/http_session.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"
#include "libmedia_transfer_protocol/libhttp/packet.h"
#include "libmedia_transfer_protocol/libhttp/http_request.h"
#include "libmedia_transfer_protocol/libhttp/tcp_handler.h"

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		class HttpServer : public sigslot::has_slots<> //: public   TcpHandler
		{
		public:
			explicit HttpServer();
			virtual ~HttpServer();

		public:

			bool Startup(const std::string &ip, uint16_t port);

			

			sigslot::signal1<  TcpSession*> SignalOnNewConnection;
			sigslot::signal1<TcpSession*> SignalOnDestory;
			sigslot::signal1<  TcpSession*> SignalOnSent;
			sigslot::signal1<  TcpSession *> SignalOnSentNextChunk;
			sigslot::signal3<  TcpSession *, const  std::shared_ptr<HttpRequest>, const std::shared_ptr<Packet>> SignalOnRequest;
		public:
			rtc::Thread* signaling_thread() { return tcp_server_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return tcp_server_->signaling_thread(); }
			rtc::Thread* worker_thread() { return tcp_server_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return tcp_server_->worker_thread(); }
			rtc::Thread* network_thread() { return tcp_server_->network_thread(); }
			const rtc::Thread* network_thread() const { return tcp_server_->network_thread(); }
		public:
			void InitSocketSignals();


			void OnNewConnection(TcpSession* conn);
			void OnDestory(TcpSession* conn);
			void OnRecv(TcpSession* conn, const rtc::CopyOnWriteBuffer& data);
			void OnSent(TcpSession* conn);

			//sigslot::signal1<TcpSession*> SignalOnNewConnection;
			//sigslot::signal1<TcpSession*> SignalOnDestroy;
			//sigslot::signal2<TcpSession*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			//sigslot::signal1<TcpSession*> SignalOnSent;
			/*
			sigslot::signal1<  TcpSession *> SignalOnSent;
			sigslot::signal1<  TcpSession *> SignalOnSentNextChunk;
			sigslot::signal3<  TcpSession *,  const  std::shared_ptr<HttpRequest> , const std::shared_ptr<Packet>> SignalOnRequest;
			
			*/


			//HttpContext
			//void OnSent(TcpSession *conn);
			void OnSentNextChunk(TcpSession *conn);
			void OnRequest(TcpSession *conn, const  std::shared_ptr<HttpRequest> http_request, const std::shared_ptr<Packet> packet);
		private:
			std::unique_ptr<TcpServer>					tcp_server_;


			//std::map<rtc::Socket*, std::unique_ptr<libhttp::TcpSession>>						http_sessions_;

			//std::unique_ptr<libcross_platform_collection_render::AudioCapture>						audio_play_;
			//libcross_platform_collection_render::cvideo_renderer * callback_ = nullptr;;

		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
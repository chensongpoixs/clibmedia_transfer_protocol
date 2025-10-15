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
				   date:  2025-10-05



 ******************************************************************************/


#ifndef _C_LIBRTSP_RTSP_SESSION_H_
#define _C_LIBRTSP_RTSP_SESSION_H_

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#ifdef WIN32
#include "rtc_base/win32_socket_server.h"
#include <vcruntime.h>
#endif
#include "libp2p_peerconnection/csession_description.h"
#include "libmedia_transfer_protocol/rtp_video_frame_assembler.h"
#include "libmedia_codec/video_codecs/h264_decoder.h"

////////////////////
#include "libcross_platform_collection_render/video_render/cvideo_render_factory.h"
#include "libcross_platform_collection_render/video_render/cvideo_render.h"
#include "libcross_platform_collection_render/track_capture/ctrack_capture.h"
#include <cstdint>
extern "C" {


#include "libavutil/error.h"
}


#include "libmedia_codec/video_codecs/nal_parse_factory.h"
namespace libmedia_transfer_protocol
{
	namespace librtsp {

		enum State {
			NOT_CONNECTED,
			RESOLVING,
			CONNECTING,
			CONNECTED,
			SIGNING_OUT_WAITING,
			SIGNING_OUT,
		};
//#pragma pack(push, 1)// 保存当前对齐状态，并设置新的对齐为1字节
		typedef struct  RtspMagic
		{//字节对齐
			uint8_t  magic_:8; // tcp中rtsp记录rtp和rtcp的包标记
			uint8_t channel_:8; //通道id

			uint16_t length_:16;
			
		} ;
//#pragma pack(pop) // 恢复之前保存的对齐状态
		class RtspSession : public   sigslot::has_slots<>
		{
		public:
			
			explicit RtspSession(rtc::Thread* network_thread, rtc::Thread* work_thread);
			virtual ~RtspSession();

		public:


			bool Play(const std::string & url);
			bool Push(const std::string & url);


			//rtc::Thread* network_thread() const {
			//	return network_thread_.get() ;
			//}


			//void RegisterDecodeCompleteCallback(libcross_platform_collection_render::cvideo_renderer * callback)
			//{
			//	callback_ = callback;
			//
			//	h264_decoder_.RegisterDecodeCompleteCallback(callback);
			//}


			sigslot::signal4<libmedia_codec::VideoCodecType  ,int32_t ,  int32_t  , int32_t  > SignalInitDeocder;
			sigslot::signal1<const libmedia_transfer_protocol::RtpPacketReceived&   > SignalRtpPacket;
		public:

			// rtsp 协议
			// 请求rtsp服务支持那些指令
			void SendOptions(rtc::Socket* socket);
			// 请求视频流
			void SendDescribe(rtc::Socket* socket);

			//设置通道连接方式
			void SendSetup(rtc::Socket* socket);

			void SendPlay(rtc::Socket* socket);

		public:
			void HandlerContentType(rtc::Socket* socket, std::vector<std::string> data);
			void HandlerSession(rtc::Socket* socket, std::vector<std::string> data);
		public:

			void Connect();


		public:


			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket );

			void OnResolveResult(rtc::AsyncResolverInterface* resolver);

			void DoConnect();

		public:
			// socket 
			void InitSocketSignals();
			bool ConnectControlSocket();;
			void Close();

			std::string BuildAuthorization(const std::string & method);
		private:
			rtc::Thread *        network_thread_;
			rtc::Thread *    work_thread_;

			std::string			      protocol_name_;
			std::string				  user_name_;//用户名（如"admin"），需与设备配置一致
			std::string				  password_;
			std::vector<std::string>  params_;
			std::string				  url_params_;
			
			int32_t                   cseq_ = 1;

			std::string               realm_; //服务器定义的认证域，不同设备可能不同（如"IP Camera(FB997)"）
			std::string				  nonce_; //服务器提供的随机数（如"2d9..."），用于增强安全性
			std::string               response_;//HA1 = MD5(username:realm:password)

			rtc::Buffer               recv_buffer_;
			int32_t					  recv_buffer_size_; //缓存数据大小
			std::string               session_id_; //回话id

			std::string        stream_url_;
			rtc::SocketAddress server_address_;
			rtc::AsyncResolver* resolver_;
			std::unique_ptr<rtc::Socket> control_socket_;
			//std::unique_ptr< rtc::PhysicalSocketServer> physical_socket_server_;
			State				state_ = NOT_CONNECTED;

		
			libp2p_peerconnection::SessionDescription   session_description_;

			std::list<std::string>					track_control_;

			std::map<std::string,   void (RtspSession::*)(rtc::Socket* socket , std::vector<std::string>)>        callback_map_;
		
			bool									decoder_init_ = false;
		//	RtpVideoFrameAssembler                  rtp_video_frame_assembler_;
			//libmedia_codec::H264Decoder              h264_decoder_;


			// callback image 
			//libcross_platform_collection_render::cvideo_renderer * callback_ = nullptr;;

		//	std::unique_ptr<libmedia_codec::NalParseInterface>             nal_parse_;
		};

	}

}



#endif // _C_LIBRTSP_RTSP_SESSION_H_


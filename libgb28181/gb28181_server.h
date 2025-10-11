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



 ******************************************************************************/

#ifndef _C_LIBGB28181_SERVER_H_
#define _C_LIBGB28181_SERVER_H_

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
namespace  libmedia_transfer_protocol {
	namespace libgb28181
	{
		class Gb28181Server : public   sigslot::has_slots<>
		{
		public:
			explicit Gb28181Server();
			virtual ~Gb28181Server();

		public:

			bool Startup(const std::string &ip, uint16_t port);

			void RegisterDecodeCompleteCallback(libcross_platform_collection_render::cvideo_renderer * callback)
			{
				callback_ = callback;

				//h264_decoder_.RegisterDecodeCompleteCallback(callback);
			}
		public:
			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }
		public:
			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);

		private:
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;

			rtc::SocketAddress server_address_;
			
			std::unique_ptr<rtc::Socket> control_socket_;
			rtc::AsyncResolver* resolver_;


			std::map<rtc::Socket*, std::unique_ptr<libgb28181::Gb28181Session>>						gb28181_sessions_;

			//std::unique_ptr<libcross_platform_collection_render::AudioCapture>						audio_play_;
			libcross_platform_collection_render::cvideo_renderer * callback_ = nullptr;;

		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
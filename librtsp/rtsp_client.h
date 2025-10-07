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
				   date:  2025-10-07



 ******************************************************************************/


#ifndef _C_LIBRTSP_RTSP_CLIENT_H_
#define _C_LIBRTSP_RTSP_CLIENT_H_

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
namespace  libmedia_transfer_protocol {

	namespace librtsp
	{
		class RtspClient : public sigslot::has_slots<>
		{
		public:
			RtspClient();
			~RtspClient();

		public:
			bool Open(const std::string & url);

			void RegisterDecodeCompleteCallback(libcross_platform_collection_render::cvideo_renderer * callback)
			{
				callback_ = callback;

				//h264_decoder_.RegisterDecodeCompleteCallback(callback);
			}

			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }

		public:
			void init(libmedia_codec::VideoCodecType  codec_type, int32_t ssrc, int32_t width, int32_t height);
			void OnRtpPacket(const libmedia_transfer_protocol:: RtpPacketReceived& packet);
		private:
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;
			std::unique_ptr<RtspSession>			rtsp_session_;

			libcross_platform_collection_render::cvideo_renderer * callback_ = nullptr;;

			std::unique_ptr<VideoReceiveStream>					video_receive_stream_;

			std::unique_ptr<libmedia_transfer_protocol::RtpStreamReceiverController>       rtp_stream_receive_controller_;
		
		
			
		
		};
	}
}

#endif // _C_LIBRTSP_RTSP_CLIENT_H_
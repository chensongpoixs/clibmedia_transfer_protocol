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
				   date:  2025-10-09



 ******************************************************************************/

#ifndef _C_LIBMPEG_DECODER_H_
#define _C_LIBMPEG_DECODER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_codec/encoded_frame.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_codec/encoded_image.h"
// //////////////////
//#include "libcross_platform_collection_render/video_render/cvideo_render_factory.h"
//#include "libcross_platform_collection_render/video_render/cvideo_render.h"
//#include "libcross_platform_collection_render/track_capture/ctrack_capture.h"
//#include "libmedia_transfer_protocol/rtp_packet_sink_interface.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
//#include "libmedia_codec/video_codec_type.h"
//#include "libmedia_codec/video_codecs/h264_decoder.h"
//#include "libmedia_codec/video_codecs/nal_parse_factory.h"
//#include "libmedia_transfer_protocol/rtp_stream_receiver_controller.h"
//#include "libmedia_transfer_protocol/librtsp/rtsp_session.h"
//#include "libmedia_transfer_protocol/video_receive_stream.h"
//#include "libp2p_peerconnection/connection_context.h"
//#include "libmedia_transfer_protocol/libgb28181/gb28181_session.h"
namespace  libmedia_transfer_protocol {
	class VideoReceiveStream;

	namespace libmpeg
	{
		
		class MpegDecoder : public   sigslot::has_slots<>
		{
		public:
			MpegDecoder();
			~MpegDecoder();

		public:

			// libmedia_codec::EncodedImage  image
			int parse(const uint8_t *data, int32_t len);


		public:
			sigslot::signal1<libmedia_codec::EncodedImage> SignalRecvVideoFrame;
			sigslot::signal1<rtc::CopyOnWriteBuffer> SignalRecvAudioFrame;
			//void RegisterDecodeCompleteCallback(VideoReceiveStream * callback)
			//{
			//	callback_ = callback;
			//}
		public:

			uint8_t*                h264_stream_;
			int32_t									stream_len_;
			int32_t                 read_byte_;
			// VideoReceiveStream *     callback_ ;
		};


	}
}


#endif // _C_LIBMPEG_DECODER_H_
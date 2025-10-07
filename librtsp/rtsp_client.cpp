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

#include "libmedia_transfer_protocol/librtsp/rtsp_client.h"

namespace  libmedia_transfer_protocol {

	namespace librtsp
	{
		 

		RtspClient::RtspClient()
			: context_( libp2p_peerconnection::ConnectionContext::Create())
			, rtsp_session_(std::make_unique<RtspSession>(context_->network_thread(), context_->worker_thread()))
			, rtp_stream_receive_controller_(nullptr)
			
		{
			context_->worker_thread()->PostTask(RTC_FROM_HERE, [this]() {
				rtp_stream_receive_controller_ = std::make_unique<libmedia_transfer_protocol::RtpStreamReceiverController>();
			});
			rtsp_session_->SignalInitDeocder.connect(this, &RtspClient::init);
			rtsp_session_->SignalRtpPacket.connect(this, &RtspClient::OnRtpPacket);
		}

		 RtspClient::~RtspClient()
		{
		}

		bool  RtspClient::Open(const std::string & url)
		{
			rtsp_session_->Play(url);
			return false;
		}

		void  RtspClient::init(libmedia_codec::VideoCodecType codec_type, int32_t ssrc, int32_t width, int32_t height)
		{
			context_->worker_thread()->PostTask(RTC_FROM_HERE, [this, codec_type, ssrc, width, height]() {
				video_receive_stream_ = std::make_unique<VideoReceiveStream>();
				video_receive_stream_->RegisterDecodeCompleteCallback(callback_);
				video_receive_stream_->init(codec_type, width, height);

				//;
				rtp_stream_receive_controller_->AddSink(ssrc, video_receive_stream_.get());
			});
			
		}

		void  RtspClient::OnRtpPacket(const libmedia_transfer_protocol::RtpPacketReceived & packet)
		{
			context_->worker_thread()->PostTask(RTC_FROM_HERE, [this, packet]() {
				rtp_stream_receive_controller_->OnRtpPacket(packet);
			});
			
		}



	}
}


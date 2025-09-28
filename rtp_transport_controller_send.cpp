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
				   date:  2025-09-28



 ******************************************************************************/


#include "libmedia_transfer_protocol/rtp_transport_controller_send.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/congestion_controller/goog_cc_network_controller.h"
namespace libmtp
{
	RtpTransportControllerSend::RtpTransportControllerSend(webrtc::Clock*clock
		, PacingController::PacketSender * packet_sender, webrtc::TaskQueueFactory* task_queue_factory)
		: clock_(clock)
		, task_queue_pacer_(std::make_unique<TaskQueuePacedSender>(clock, 
			packet_sender, task_queue_factory, webrtc::TimeDelta::Millis(1)))
		, task_queue_(task_queue_factory->CreateTaskQueue("rtp_send_task_queue", webrtc::TaskQueueFactory::Priority::NORMAL))
	{
		task_queue_pacer_->EnsureStarted();
	}

	RtpTransportControllerSend::~RtpTransportControllerSend()
	{}
	void RtpTransportControllerSend::EnqueuePacket(std::vector<std::unique_ptr<RtpPacketToSend>> packets)
	{
		task_queue_pacer_->EnqueuePackets(std::move(packets));
	}



	void RtpTransportControllerSend::OnTransportFeedback(const rtcp::TransportFeedback &feedback)
	{
		webrtc::Timestamp feedback_time = webrtc::Timestamp::Millis(clock_->TimeInMilliseconds());

		task_queue_.PostTask([this, feedback, feedback_time]() {
			transport_feedback_adapter_.ProcessTransportFeedback(feedback, feedback_time);
		});
		
	}
	void RtpTransportControllerSend::OnAddPacket(const libmedia_transfer_protocol::RtpPacketSendInfo & send_info)
	{
		webrtc::Timestamp creation_time = webrtc::Timestamp::Millis(clock_->TimeInMilliseconds());

		task_queue_.PostTask([this, creation_time, send_info]() {
			transport_feedback_adapter_.AddPacket(  creation_time, send_info.length, send_info);
		});
	}
	void RtpTransportControllerSend::OnSentPacket(const rtc::SentPacket& sent_packet)
	{
		task_queue_.PostTask([this, sent_packet]() {
			transport_feedback_adapter_.ProcessSentPacket(sent_packet);
		});
	}
	void RtpTransportControllerSend::OnNetworkOk(bool network_ok)
	{
		RTC_LOG_F(LS_INFO) << "network state = " << network_ok;
		libice::NetworkAvailability  msg;
		msg.at_time = webrtc::Timestamp::Millis(clock_->TimeInMilliseconds());
		msg.network_available = network_ok;

		task_queue_.PostTask([this, msg]() {
			
			if (network_ok_ == msg.network_available)
			{
				return;
			}
			//网络状态发生变化
			network_ok_ = msg.network_available;
			if (controller_)
			{

			}
			else
			{
				MaybeCreateController();
			}
		});
	}
	void RtpTransportControllerSend::MaybeCreateController()
	{
		if (!network_ok_)
		{
			return;
		}
		controller_ = std::make_unique<GoogCcNetworkController>();
	}
}
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



#ifndef _C_TRANSPORT_CONTROLLER_SEND_H_
#define _C_TRANSPORT_CONTROLLER_SEND_H_

#include "media_config.h"
#include "rtc_base/fake_clock.h"
#include "system_wrappers/include/clock.h"
#include "libmedia_transfer_protocol/pacing/task_queue_paced_sender.h"
#include "libmedia_transfer_protocol/network_controller.h"
#include "libmedia_transfer_protocol/rtp/transport_feedback_adapter.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/transport_feedback.h"
#include "rtc_base/network/sent_packet.h"
namespace  libmtp
{
	class RtpTransportControllerSend
	{
	public:
		RtpTransportControllerSend(webrtc::Clock*clock, 
			PacingController::PacketSender * packet_sender,
			webrtc::TaskQueueFactory* task_queue_factory);
		virtual ~RtpTransportControllerSend();


	public:
		void EnqueuePacket(std::vector<std::unique_ptr<RtpPacketToSend>> packets);



		void OnTransportFeedback(const rtcp::TransportFeedback &feedback);
	public:



		void OnAddPacket(const libmedia_transfer_protocol::RtpPacketSendInfo & send_info);
		void OnSentPacket(const rtc::SentPacket& sent_packet);
		void OnNetworkOk(bool  network_ok);


		
	private:

		void MaybeCreateController();
	private:


		webrtc::Clock* clock_;

		std::unique_ptr<NetworkControllerInterface>   controller_;

		std::unique_ptr<TaskQueuePacedSender>  task_queue_pacer_;
		bool     network_ok_ = false;
		rtc::TaskQueue task_queue_;
		TransportFeedbackAdapter    transport_feedback_adapter_;
		
	};
}


#endif //_C_TRANSPORT_CONTROLLER_SEND_H_

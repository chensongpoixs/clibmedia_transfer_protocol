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
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_receiver.h"
namespace  libmtp
{
	class RtpTransportControllerSend  : public TransportFeedbackObserver
		//, public RtcpBandwidthObserver 
		//, public  sigslot::has_slots<>
	{
	public:
		RtpTransportControllerSend(webrtc::Clock*clock, 
			PacingController::PacketSender * packet_sender,
			webrtc::TaskQueueFactory* task_queue_factory);
		virtual ~RtpTransportControllerSend() override;


	public:

		sigslot::signal3<const ReportBlockList& , int64_t, int64_t> SignalOnNetworkInfo;
		sigslot::signal2<RtpTransportControllerSend*, const libice::TargetTransferRate&> SignalTargetTransferRate;
	public:
		void EnqueuePacket(std::vector<std::unique_ptr<RtpPacketToSend>> packets);



		
	public:

	//	virtual void OnAddPacket(const RtpPacketSendInfo& packet_info) = 0;
	//	virtual void OnTransportFeedback(const rtcp::TransportFeedback& feedback) = 0;

		// TransportFeedbackObserver  callback
		void OnAddPacket(const libmedia_transfer_protocol::RtpPacketSendInfo & send_info) override;
		void OnTransportFeedback(const rtcp::TransportFeedback &feedback) override;




		void OnReceivedRtcpReceiverReportBlocks(const ReportBlockList& report_blocks,
			int64_t now_ms);
		//RtcpBandwidthObserver  callback 
		//void OnReceivedEstimatedBitrate(uint32_t bitrate) override; 
		//void OnReceivedRtcpReceiverReport(
		//const ReportBlockList& report_blocks,
		//int64_t rtt,
		//int64_t now_ms) override;
	public:
		void OnSentPacket(const rtc::SentPacket& sent_packet);
		void OnNetworkOk(bool  network_ok);

		void OnRttUpdate(int64_t rtt_ms, webrtc::Timestamp at_time);// override;
		
	private:

		void MaybeCreateController();

		//void PostUpdate()
		void PostUpdates(libice::NetworkControlUpdate update) ;
	private:


		webrtc::Clock* clock_;

		std::unique_ptr<NetworkControllerInterface>   controller_;

		std::unique_ptr<TaskQueuePacedSender>  task_queue_pacer_;
		bool     network_ok_ = false;
		rtc::TaskQueue task_queue_;
		TransportFeedbackAdapter    transport_feedback_adapter_;

		//保存RR 的信息结构
		std::map<uint32_t, RTCPReportBlock> last_report_blocks_;
		//上一次接受RTCP中RR包的时间
		webrtc::Timestamp last_report_block_time_ = webrtc::Timestamp::MinusInfinity();//
		NetworkControllerConfig                     controller_config_;
		
	};
}


#endif //_C_TRANSPORT_CONTROLLER_SEND_H_

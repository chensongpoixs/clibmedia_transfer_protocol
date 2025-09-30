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
			// feedback 网络反馈包转换为应用的数据结构 TransportFeedback ==> TransportPacketsFeedback
			absl::optional<libice::TransportPacketsFeedback> feedback_msg=	
				transport_feedback_adapter_.ProcessTransportFeedback(feedback, feedback_time);
			if (feedback_msg &&controller_)
			{
				controller_->OnTransportPacketsFeedback(*feedback_msg);
			}
		});
		
	}
	void  RtpTransportControllerSend::OnReceivedRtcpReceiverReportBlocks(
		const ReportBlockList& report_blocks, int64_t now_ms)
	{
		if (report_blocks.empty())
		{
			return;
		}

		int total_packets_lost_delta = 0;
		int total_packets_delta = 0;
		// 基于RR 统计数据计算掉包率 更新掉包网络带宽评估
		// Compute the packet loss from all report blocks.
		for (const RTCPReportBlock& report_block : report_blocks) 
		{
			auto it = last_report_blocks_.find(report_block.source_ssrc);
			if (it != last_report_blocks_.end()) 
			{
				uint32_t number_of_packets = report_block.extended_highest_sequence_number -
					it->second.extended_highest_sequence_number;
				total_packets_delta += number_of_packets;
				auto lost_delta = report_block.packets_lost - it->second.packets_lost;
				total_packets_lost_delta += lost_delta;
			}
			last_report_blocks_[report_block.source_ssrc] = report_block;
		}
		// Can only compute delta if there has been previous blocks to compare to. If
		// not, total_packets_delta will be unchanged and there's nothing more to do.
		if (!total_packets_delta)
		{
			return;
		}
		int packets_received_delta = total_packets_delta - total_packets_lost_delta;
		// To detect lost packets, at least one packet has to be received. This check
		// is needed to avoid bandwith detection update in
		// VideoSendStreamTest.SuspendBelowMinBitrate

		if (packets_received_delta < 1)
		{
			return;
		}
		webrtc::Timestamp now = webrtc::Timestamp::Millis(now_ms);
		libice::TransportLossReport msg;
		msg.packets_lost_delta = total_packets_lost_delta;
		msg.packets_received_delta = packets_received_delta;
		msg.receive_time = now;
		msg.start_time = last_report_block_time_;
		msg.end_time = now;
		last_report_block_time_ = now;
		task_queue_.PostTask([this, msg]() {
			if (controller_)
			{
				controller_->OnTransportLossReport(msg);
			}
		});
		/*if (controller_)
			PostUpdates(controller_->OnTransportLossReport(msg));*/
		
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
				//设置掉包起始码流 start , min , max rate 
				libice::TargetRateConstraints constraints;
				constraints.starting_rate = webrtc::DataRate::KilobitsPerSec(3000);
				constraints.min_data_rate = webrtc::DataRate::KilobitsPerSec(3000);
				constraints.max_data_rate = webrtc::DataRate::KilobitsPerSec(100000);
				controller_->OnTargetRateConstraints(constraints);
			}
		});
	}
	void RtpTransportControllerSend::OnRttUpdate(int64_t rtt_ms, webrtc::Timestamp at_time)
	{
		task_queue_.PostTask([this, rtt_ms, at_time]() {
			if (controller_)
			{
				controller_->OnRttUpdate(rtt_ms, at_time);
			}
		});
	}
	void RtpTransportControllerSend::MaybeCreateController()
	{
		if (!network_ok_)
		{
			return;
		}
		controller_ = std::make_unique<GoogCcNetworkController>(controller_config_);
	}
}
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
				   date:  2025-09-23



 ******************************************************************************/

#include "libmedia_transfer_protocol/rtp/transport_feedback_adapter.h"
#include "rtc_base/logging.h"

namespace libmedia_transfer_protocol {

	constexpr webrtc::TimeDelta kSendTimeHistoryWindow = webrtc::TimeDelta::Seconds(60);

	TransportFeedbackAdapter::TransportFeedbackAdapter()
	{

	}
	TransportFeedbackAdapter::~TransportFeedbackAdapter()
	{

	}
	void TransportFeedbackAdapter::AddPacket(webrtc::Timestamp creation_time,
		size_t overhead_bytes, const libmedia_transfer_protocol::RtpPacketSendInfo& send_info)
	{

		PacketFeedback packet;
		packet.creation_time = creation_time;
		packet.sent.sequence_number = seq_num_unwrapper_.Unwrap( send_info.transport_sequence_number);
		packet.sent.size = webrtc::DataSize::Bytes(send_info.length + overhead_bytes);
		packet.sent.audio = (send_info.packet_type == libmedia_transfer_protocol::RtpPacketMediaType::kAudio ||
			send_info.packet_type == libmedia_transfer_protocol::RtpPacketMediaType::kVideo);
		
		//探测包的数据增加
		packet.sent.pacing_info = send_info.pacing_info;
		while (!history_.empty() && creation_time - history_.begin()->second.creation_time >
			kSendTimeHistoryWindow) {
			// TODO(sprang): Warn if erasing (too many) old items?
			//if (history_.begin()->second.sent.sequence_number > last_ack_seq_num_)
			//	in_flight_.RemoveInFlightPacketBytes(history_.begin()->second);
			history_.erase(history_.begin());
		}
		history_.insert(std::make_pair(packet.sent.sequence_number, packet));

	}
	absl::optional<libice::SentPacket>  TransportFeedbackAdapter::ProcessSentPacket(const rtc::SentPacket& send_packet)
	{
		auto send_time = webrtc::Timestamp::Millis(send_packet.send_time_ms);
		// 获取缓存中发送包记录
		int64_t unwrapped_seq_num = seq_num_unwrapper_.Unwrap(send_packet.packet_id);
		auto it = history_.find(unwrapped_seq_num);
		if (it != history_.end())
		{
			bool packet_retransmit = it->second.sent.send_time.IsFinite();
			it->second.sent.send_time = send_time;

			last_send_time_ = std::max(last_send_time_, send_time);
			if (!packet_retransmit)
			{
				return it->second.sent;
			}
		}
		return absl::nullopt;
	}
	absl::optional<libice::TransportPacketsFeedback> TransportFeedbackAdapter::ProcessTransportFeedback(
		const rtcp::TransportFeedback & feedback, webrtc::Timestamp feedback_time)
	{
		//当前feedback中是否有反馈包数量 
		if (feedback.GetPacketStatusCount() == 0)
		{
			RTC_LOG_F(LS_WARNING) << "Empty rtp packet in transport feedback";
			return absl::nullopt;
		}


		libice::TransportPacketsFeedback msg;
		msg.feedback_time = feedback_time;
		msg.packet_feedbacks = ProcessTransportFeedbackInner(feedback, feedback_time);

		if (msg.packet_feedbacks.empty())
		{
			return absl::nullopt;
		}
		auto it = history_.find(last_ack_seq_num_);
		if (it != history_.end())
		{
			msg.first_unacked_send_time = it->second.sent.send_time;
		}
		return msg;
		//return absl::optional<libice::TransportPacketsFeedback>();
	}
	std::vector<libice::PacketResult> TransportFeedbackAdapter::ProcessTransportFeedbackInner(
		const rtcp::TransportFeedback & feedback, webrtc::Timestamp feedback_time)
	{
		size_t   failed_lookups= 0;
		webrtc::TimeDelta   packet_offset = webrtc::TimeDelta::Zero();
		// 第一次收到feedback包
		if (last_timestamp_.IsInfinite())
		{
			current_offset_ = feedback_time;
		}
		else
		{
			webrtc::TimeDelta delta = feedback.GetBaseDelta(last_timestamp_).RoundDownTo(webrtc::TimeDelta::Millis(1));
			if (delta < webrtc::Timestamp::Zero() - current_offset_)
			{
				RTC_LOG_F(LS_WARNING) << "Unexpected feedback timestamp received.";
				current_offset_ = feedback_time;
			}
			else
			{
				//叠加时间
				current_offset_ += delta;
			}
		}
		// 更新时间 base_time_ticks_
		last_timestamp_ = feedback.GetBaseTime();


		std::vector<libice::PacketResult> packet_result_vector;
		packet_result_vector.reserve(feedback.GetPacketStatusCount());

		for (const auto & packet : feedback.GetAllPackets())
		{
			int64_t seq_num = seq_num_unwrapper_.Unwrap(packet.sequence_number());
			if (seq_num > last_ack_seq_num_)
			{
				last_ack_seq_num_ = seq_num;
			}

			//查找seq_num  是否发送历史记录中存在

			auto it = history_.find(seq_num);
			if (it == history_.end())
			{
				++failed_lookups;
				continue;
			}

			// RTP 数据包还没有标记发送
			if (it->second.sent.send_time.IsInfinite())
			{
				RTC_DLOG(LS_ERROR)
					<< "Received feedback before packet was indicated as sent";
				continue;
			}


			PacketFeedback packet_feedback = it->second;
			// 计算每个rtp包的达到时间， 转换成发送端的时间
			if (packet.received()) {
				packet_offset += packet.delta();
				packet_feedback.receive_time =
					current_offset_ + packet_offset.RoundDownTo(webrtc::TimeDelta::Millis(1));
				// Note: Lost packets are not removed from history because they might be
				// reported as received by a later feedback.
				history_.erase(it);
			}

			libice::PacketResult   result;
			result.sent_packet = packet_feedback.sent;
			result.receive_time = packet_feedback.receive_time;
			packet_result_vector.push_back(result);
		}


		return packet_result_vector;
		//return std::vector<libice::PacketResult>();
	}
}
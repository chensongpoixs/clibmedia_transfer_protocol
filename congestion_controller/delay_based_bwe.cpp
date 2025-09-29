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
				   date:  2025-09-29


				   基于延迟的带宽估计



				   InterArrivalDelta 计算组包延迟差
				   TredlineEstimator 延迟趋势估计



				   AimdRateControl  码率控制  <----  LinkCapacityEstiomator  链路容量估计

 ******************************************************************************/

#include "libmedia_transfer_protocol/congestion_controller/delay_based_bwe.h"
#include "rtc_base/logging.h"
namespace libmtp
{
	//流超时时间设置
	constexpr webrtc::TimeDelta kStreamTimeOut = webrtc::TimeDelta::Seconds(2);

	// Used with field trial "WebRTC-Bwe-NewInterArrivalDelta/Enabled/
	constexpr webrtc::TimeDelta kSendTimeGroupLength = webrtc::TimeDelta::Millis(5);
	DelayBasedBwe::Result::Result()
		: updated(false),
		probe(false),
		target_bitrate(webrtc::DataRate::Zero()),
		recovered_from_overuse(false),
		backoff_in_alr(false) {}


	DelayBasedBwe::DelayBasedBwe()
		: video_inter_arrival_delta_(nullptr)
		, last_seen_packet_(webrtc::Timestamp::MinusInfinity())
	{
	}
	DelayBasedBwe::~DelayBasedBwe()
	 {
	 }
	DelayBasedBwe::Result DelayBasedBwe::IncomingPacketFeedbackVector(const libice::TransportPacketsFeedback & msg, bool in_alr)
	{
		//按照接受时间排序
		auto packet_feedback_vector = msg.SortedByReceiveTime();

		if (packet_feedback_vector.empty()) {
			RTC_LOG(LS_WARNING) << "Very late feedback received.";
			return DelayBasedBwe::Result();
		}

		bool delayed_feedback = true;
		bool recovered_from_overuse = false;
		for (const auto& packet_feedback : packet_feedback_vector) {
			delayed_feedback = false;
			IncomingPacketFeedback(packet_feedback, msg.feedback_time);
			//if (prev_detector_state == BandwidthUsage::kBwUnderusing &&
			//	active_delay_detector_->State() == BandwidthUsage::kBwNormal) {
			//	recovered_from_overuse = true;
			//}
			//prev_detector_state = active_delay_detector_->State();
		}
		return Result();
	}

	void DelayBasedBwe::IncomingPacketFeedback(const libice::PacketResult & packet_feedback, webrtc::Timestamp at_time)
	{
		// Reset if the stream has timed out.
		if (last_seen_packet_.IsInfinite() ||
			at_time - last_seen_packet_ > kStreamTimeOut)
		{
			 // 创建延迟梯度
			video_inter_arrival_delta_ = std::make_unique<InterArrivalDelta>(kSendTimeGroupLength);
			audio_inter_arrival_delta_ = std::make_unique<InterArrivalDelta>(kSendTimeGroupLength);
			 
			 
			// 延迟趋势
			//video_delay_detector_.reset(
			//	new TrendlineEstimator(key_value_config_, network_state_predictor_));
			//audio_delay_detector_.reset(
			//	new TrendlineEstimator(key_value_config_, network_state_predictor_));
			//active_delay_detector_ = video_delay_detector_.get();
		}
		last_seen_packet_ = at_time;

		// As an alternative to ignoring small packets, we can separate audio and
		// video packets for overuse detection.
		//DelayIncreaseDetectorInterface* delay_detector_for_packet =
		//	video_delay_detector_.get();
		//if (separate_audio_.enabled) {
		//	if (packet_feedback.sent_packet.audio) {
		//		delay_detector_for_packet = audio_delay_detector_.get();
		//		audio_packets_since_last_video_++;
		//		if (audio_packets_since_last_video_ > separate_audio_.packet_threshold &&
		//			packet_feedback.receive_time - last_video_packet_recv_time_ >
		//			separate_audio_.time_threshold) {
		//			active_delay_detector_ = audio_delay_detector_.get();
		//		}
		//	}
		//	else {
		//		audio_packets_since_last_video_ = 0;
		//		last_video_packet_recv_time_ =
		//			std::max(last_video_packet_recv_time_, packet_feedback.receive_time);
		//		active_delay_detector_ = video_delay_detector_.get();
		//	}
		//}
		webrtc::DataSize packet_size = packet_feedback.sent_packet.size;

		 
		webrtc::TimeDelta send_delta = webrtc::TimeDelta::Zero();
		webrtc::TimeDelta recv_delta = webrtc::TimeDelta::Zero();
		int size_delta = 0;

		InterArrivalDelta* inter_arrival_for_packet =
			(/*separate_audio_.enabled && */packet_feedback.sent_packet.audio)
			? video_inter_arrival_delta_.get()
			: audio_inter_arrival_delta_.get();
		bool calculated_deltas = inter_arrival_for_packet->ComputeDeltas(
			packet_feedback.sent_packet.send_time, packet_feedback.receive_time,
			at_time, packet_size.bytes(), &send_delta, &recv_delta, &size_delta);


		RTC_LOG(LS_INFO) << "video inter arrival send_delta:" << webrtc::ToString(send_delta) << ", recv_delta:" << webrtc::ToString(recv_delta )<< ", size_delta:" <<  size_delta;
		//delay_detector_for_packet->Update(
		//	recv_delta.ms(), send_delta.ms(),
		//	packet_feedback.sent_packet.send_time.ms(),
		//	packet_feedback.receive_time.ms(), packet_size.bytes(),
		//	calculated_deltas);
		 
		 
	}

}
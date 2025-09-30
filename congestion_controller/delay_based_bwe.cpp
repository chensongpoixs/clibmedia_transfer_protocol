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
		, video_delay_detector_(std::make_unique<TrendlineEstimator>(nullptr))
		, audio_inter_arrival_delta_(nullptr)
		, audio_delay_detector_(std::make_unique<TrendlineEstimator>(nullptr))
		, last_seen_packet_(webrtc::Timestamp::MinusInfinity())
		, rate_control_()
		, uma_recorded_(false)
		, prev_bitrate_(webrtc::DataRate::Zero())
		, has_once_detected_overuse_(false)
		, prev_state_(BandwidthUsage::kBwNormal)
		, use_new_inter_arrival_delta_(false)
		, alr_limited_backoff_enabled_(false)
	{
	}
	DelayBasedBwe::~DelayBasedBwe()
	 {
	 }
	DelayBasedBwe::Result DelayBasedBwe::IncomingPacketFeedbackVector(
		const libice::TransportPacketsFeedback & msg, 
		absl::optional<webrtc::DataRate> acked_bitrate,
		absl::optional<webrtc::DataRate> probe_bitrate,
		absl::optional<libice::NetworkStateEstimate> network_estimate,
		bool in_alr)
	{
		//按照接受时间排序
		auto packet_feedback_vector = msg.SortedByReceiveTime();

		if (packet_feedback_vector.empty()) {
			RTC_LOG(LS_WARNING) << "Very late feedback received.";
			return DelayBasedBwe::Result();
		}

		bool delayed_feedback = true;
		// 是否从过载中恢复过来
		bool recovered_from_overuse = false;
		// 上次状态
		BandwidthUsage prev_detector_state = video_delay_detector_->State();
		for (const auto& packet_feedback : packet_feedback_vector) {
			delayed_feedback = false;
			IncomingPacketFeedback(packet_feedback, msg.feedback_time);
			if (prev_detector_state == BandwidthUsage::kBwUnderusing &&
				video_delay_detector_->State() == BandwidthUsage::kBwNormal) 
			{
				 // 状态恢复过来了
				recovered_from_overuse = true;
			}
			// 更新上一个状态
			prev_detector_state = video_delay_detector_->State();
		}
		rate_control_.SetInApplicationLimitedRegion(in_alr);
		rate_control_.SetNetworkStateEstimate(network_estimate);
		return MaybeUpdateEstimate(acked_bitrate, probe_bitrate,
			std::move(network_estimate),
			recovered_from_overuse, in_alr, msg.feedback_time);
	}

	void DelayBasedBwe::OnRttUpdate(int64_t rtt_ms)
	{
		rate_control_.SetRtt(webrtc::TimeDelta::Millis(rtt_ms));
	}

	void DelayBasedBwe::SetStartBitrate(webrtc::DataRate start_bitrate)
	{
		RTC_LOG(LS_INFO) << "BWE Setting start bitrate to: "
			<< webrtc::ToString(start_bitrate);
		rate_control_.SetStartBitrate(start_bitrate);
	}

	void DelayBasedBwe::IncomingPacketFeedback(
		const libice::PacketResult & packet_feedback, webrtc::Timestamp at_time)
	{
		// Reset if the stream has timed out.
		if (last_seen_packet_.IsInfinite() ||
			at_time - last_seen_packet_ > kStreamTimeOut)
		{
			 // 创建延迟梯度
			video_inter_arrival_delta_ = std::make_unique<InterArrivalDelta>(kSendTimeGroupLength);
			audio_inter_arrival_delta_ = std::make_unique<InterArrivalDelta>(kSendTimeGroupLength);
			 
			 
			// 延迟趋势
#if !TRENDLINE_ESTIMEATOR_CSV
			video_delay_detector_.reset(
			new TrendlineEstimator(nullptr/*key_value_config_, network_state_predictor_*/));
			audio_delay_detector_.reset(
				new TrendlineEstimator(nullptr/*key_value_config_, network_state_predictor_*/));
			//active_delay_detector_ = video_delay_detector_.get();
#endif 
		}
		last_seen_packet_ = at_time;

		// As an alternative to ignoring small packets, we can separate audio and
		// video packets for overuse detection.
		DelayIncreaseDetectorInterface* delay_detector_for_packet =
			video_delay_detector_.get();
		/*if (separate_audio_.enabled) {
			if (packet_feedback.sent_packet.audio) {
				delay_detector_for_packet = audio_delay_detector_.get();
				audio_packets_since_last_video_++;
				if (audio_packets_since_last_video_ > separate_audio_.packet_threshold &&
					packet_feedback.receive_time - last_video_packet_recv_time_ >
					separate_audio_.time_threshold) {
					active_delay_detector_ = audio_delay_detector_.get();
				}
			}
			else {
				audio_packets_since_last_video_ = 0;
				last_video_packet_recv_time_ =
					std::max(last_video_packet_recv_time_, packet_feedback.receive_time);
				active_delay_detector_ = video_delay_detector_.get();
			}
		}*/
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


		//RTC_LOG(LS_INFO) << "video inter arrival send_delta:" << webrtc::ToString(send_delta) << ", recv_delta:" << webrtc::ToString(recv_delta )<< ", size_delta:" <<  size_delta;
		//inter_arrival_for_packet->ComputeDeltas();
		delay_detector_for_packet->Update(
			recv_delta.ms(), send_delta.ms(),
			packet_feedback.sent_packet.send_time.ms(),
			packet_feedback.receive_time.ms(), packet_size.bytes(),
			calculated_deltas);
		 
		 
	}

	DelayBasedBwe::Result DelayBasedBwe::MaybeUpdateEstimate(
		absl::optional<webrtc::DataRate> acked_bitrate, 
		absl::optional<webrtc::DataRate> probe_bitrate,
		absl::optional<libice::NetworkStateEstimate> state_estimate, 
		bool recovered_from_overuse, bool in_alr, webrtc::Timestamp at_time)
	{
		//根据网络状态检测 动态特征发送码率
		Result  result;
		//当网络出现过载的时候
		 // Currently overusing the bandwidth.
		if (video_delay_detector_->State() == BandwidthUsage::kBwOverusing) 
		{
			
			//if (has_once_detected_overuse_ && in_alr && alr_limited_backoff_enabled_) {
			//	if (rate_control_.TimeToReduceFurther(at_time, prev_bitrate_)) {
			//		result.updated =
			//			UpdateEstimate(at_time, prev_bitrate_, &result.target_bitrate);
			//		result.backoff_in_alr = true;
			//	}
			//}
			//else 
			// 已经知道吞吐量时  码控模块可以进一步降低码流
			if (acked_bitrate &&
				rate_control_.TimeToReduceFurther(at_time, *acked_bitrate))
			{
				 // 当前码流是否更新
				result.updated =
					UpdateEstimate(at_time, acked_bitrate, &result.target_bitrate);
			}
			else if (!acked_bitrate && rate_control_.ValidEstimate() &&
				rate_control_.InitialTimeToReduceFurther(at_time)) //不知道吞吐量 时候是否可以吞吐量降低码流
			{
				// Overusing before we have a measured acknowledged bitrate. Reduce send
				// rate by 50% every 200 ms.
				// TODO(tschumim): Improve this and/or the acknowledged bitrate estimator
				// so that we (almost) always have a bitrate estimate.
				// 不知道吞吐量时候， 将当前码流下降一半
				rate_control_.SetEstimate(rate_control_.LatestEstimate() / 2, at_time);
				//码流降低了就需要更新了
				result.updated = true;
				result.probe = false;
				result.target_bitrate = rate_control_.LatestEstimate();
			}
			has_once_detected_overuse_ = true;
		}
		else {
			/*if (probe_bitrate) {
				result.probe = true;
				result.updated = true;
				result.target_bitrate = *probe_bitrate;
				rate_control_.SetEstimate(*probe_bitrate, at_time);
			}
			else {
				result.updated =
					UpdateEstimate(at_time, acked_bitrate, &result.target_bitrate);
				result.recovered_from_overuse = recovered_from_overuse;
			}*/
		}
		BandwidthUsage detector_state = video_delay_detector_->State();
		/*if ((result.updated && prev_bitrate_ != result.target_bitrate) ||
			detector_state != prev_state_) {
			DataRate bitrate = result.updated ? result.target_bitrate : prev_bitrate_;

			BWE_TEST_LOGGING_PLOT(1, "target_bitrate_bps", at_time.ms(), bitrate.bps());

			if (event_log_) {
				event_log_->Log(std::make_unique<RtcEventBweUpdateDelayBased>(
					bitrate.bps(), detector_state));
			}

			prev_bitrate_ = bitrate;
			prev_state_ = detector_state;
		}*/
		return result;

		return Result();
	}

	bool DelayBasedBwe::UpdateEstimate(webrtc::Timestamp at_time, absl::optional<webrtc::DataRate> acked_bitrate, webrtc::DataRate * target_rate)
	{
		const RateControlInput input(video_delay_detector_->State(), acked_bitrate);
		*target_rate = rate_control_.Update(&input, at_time);
		return rate_control_.ValidEstimate();
	}

}
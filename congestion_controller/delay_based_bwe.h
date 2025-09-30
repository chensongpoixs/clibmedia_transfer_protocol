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

------------------------------------------------------------------------------------------------------------

				                                    1、 包组延迟计算(InterArrivalDelta)

				   基于延迟的带宽估计 ====>>>>         2、趋势线延迟梯度估计器（TrendlineEstimator）
					
													3、AIMD码率控制器（AimdRateControl）

------------------------------------------------------------------------------------------------------------

				   InterArrivalDelta 计算组包延迟差
				   TredlineEstimator 延迟趋势估计
				     
					过载检测的  网络三种状态：  正常， 高负载， 低负载

				   AimdRateControl  码率控制  <----  LinkCapacityEstiomator  链路容量估计

 ******************************************************************************/


#ifndef _C_DELAY_BASED_BWE_
#define _C_DELAY_BASED_BWE_
#include "libmedia_transfer_protocol/media_config.h"
#include "libice/network_types.h"
#include "libmedia_transfer_protocol/network_controller.h"
#include "libmedia_transfer_protocol/congestion_controller/inter_arrival_delta.h"
#include "libmedia_transfer_protocol/congestion_controller/trendline_estimator.h"
#include <string>
#include <sstream>
#include "libmedia_transfer_protocol/remote_bitrate_estimator/aimd_rate_control.h"
namespace libmtp
{
	class DelayBasedBwe
	{
	public:
		struct Result
		{
			Result();
			~Result() = default;

			bool updated; //是否更新
			bool probe; // 
			//目标码率
			webrtc::DataRate target_bitrate = webrtc::DataRate::Zero(); // 
			bool  recovered_from_overuse;
			bool  backoff_in_alr;

			std::string ToString() const
			{
				std::stringstream cmd;

				cmd << "result updated:" << updated;
				cmd << ", target_bitrate:" << webrtc::ToString(target_bitrate);
				return cmd.str();
			}
		};
	public:
		 DelayBasedBwe();
		//DelayBasedBwe() = delete;
		DelayBasedBwe(const DelayBasedBwe&) = delete;
		DelayBasedBwe& operator=(const DelayBasedBwe&) = delete;
		virtual ~DelayBasedBwe();

	public:
		Result IncomingPacketFeedbackVector(
			const libice::TransportPacketsFeedback& msg,
			absl::optional<webrtc::DataRate> acked_bitrate,
			absl::optional<webrtc::DataRate> probe_bitrate,
			absl::optional<libice::NetworkStateEstimate> network_estimate,
			bool in_alr);

		void OnRttUpdate(int64_t rtt_ms);

		void SetStartBitrate(webrtc::DataRate start_bitrate);
	private:
		void IncomingPacketFeedback(const libice::PacketResult& packet_feedback,
			webrtc::Timestamp at_time);


		Result MaybeUpdateEstimate(
			absl::optional<webrtc::DataRate> acked_bitrate /*确认吞吐量*/,
			absl::optional<webrtc::DataRate> probe_bitrate,
			absl::optional<libice::NetworkStateEstimate> state_estimate,
			bool recovered_from_overuse,
			bool in_alr,
			webrtc::Timestamp at_time);

		// estimate exists.
		bool UpdateEstimate(webrtc::Timestamp at_time,
			absl::optional<webrtc::DataRate> acked_bitrate,
			webrtc::DataRate* target_rate /*目标码流*/);
	private:
		//std::unique_ptr<InterArrival> video_inter_arrival_;
		std::unique_ptr<InterArrivalDelta> video_inter_arrival_delta_;
		std::unique_ptr<DelayIncreaseDetectorInterface> video_delay_detector_;
		//std::unique_ptr<InterArrival> audio_inter_arrival_;
		std::unique_ptr<InterArrivalDelta> audio_inter_arrival_delta_;
		std::unique_ptr<DelayIncreaseDetectorInterface> audio_delay_detector_;
		webrtc::Timestamp last_seen_packet_;


		//Timestamp last_seen_packet_;
		bool uma_recorded_;
		// 码控模块
		AimdRateControl rate_control_;
		webrtc::DataRate prev_bitrate_;
		bool has_once_detected_overuse_;
		BandwidthUsage prev_state_;
		const bool use_new_inter_arrival_delta_;
		bool alr_limited_backoff_enabled_;
	};

}

#endif // 
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
#include "libmedia_transfer_protocol/congestion_controller/inter_arrival_delta.h"
#include "rtc_base/logging.h"
namespace libmtp
{
	static constexpr webrtc::TimeDelta kBurstDeltaThreshold = webrtc::TimeDelta::Millis(5);

	// 判断两个是否突发包 时间间隔 100ms
	static constexpr webrtc::TimeDelta kMaxBurstDuration = webrtc::TimeDelta::Millis(100);
	constexpr webrtc::TimeDelta InterArrivalDelta::kArrivalTimeOffsetThreshold;
	InterArrivalDelta::InterArrivalDelta(webrtc::TimeDelta send_time_group_length)
		: send_time_group_length_(send_time_group_length)
		,current_timestamp_group_(),
		prev_timestamp_group_(),
		num_consecutive_reordered_packets_(0) {}
	 
	InterArrivalDelta::~InterArrivalDelta()
	{
	}
	bool InterArrivalDelta::ComputeDeltas(webrtc::Timestamp send_time, 
		webrtc::Timestamp arrival_time, webrtc::Timestamp system_time, 
		size_t packet_size, webrtc::TimeDelta * send_time_delta, 
		webrtc::TimeDelta * arrival_time_delta, int * packet_size_delta)
	{
		bool calculated_deltas = false;
		// 第一个包 
		if (current_timestamp_group_.IsFirstPacket()) {
			// We don't have enough data to update the filter, so we store it until we
			// have two frames of data to process.
			current_timestamp_group_.send_time = send_time;
			current_timestamp_group_.first_send_time = send_time;
			current_timestamp_group_.first_arrival = arrival_time;
		}
		else if (current_timestamp_group_.first_send_time > send_time) {
			// Reordered packet.
			//  顺序错误 
			return false;
		}
		else if (NewTimestampGroup(arrival_time, send_time))// 是否创建新的分组
		{

			// First packet of a later send burst, the previous packets sample is ready.
			// 判断是否需要计算两个包组之间的时间差
			if (prev_timestamp_group_.complete_time.IsFinite())
			{
				// 两个分组的发送时间差
				*send_time_delta = current_timestamp_group_.send_time - prev_timestamp_group_.send_time;
				// 两个分组接受时间差
				*arrival_time_delta = current_timestamp_group_.complete_time - prev_timestamp_group_.complete_time;

				webrtc::TimeDelta system_time_delta = current_timestamp_group_.last_system_time -
					prev_timestamp_group_.last_system_time;
				// 正常是不会相差太大了 除非突变
				if (*arrival_time_delta - system_time_delta >=
					kArrivalTimeOffsetThreshold) {
					RTC_LOG(LS_WARNING)
						<< "The arrival time clock offset has changed (diff = "
						<< arrival_time_delta->ms() - system_time_delta.ms()
						<< " ms), resetting.";
					Reset();
					return false;
				}
				if (*arrival_time_delta < webrtc::TimeDelta::Zero()) {
					// The group of packets has been reordered since receiving its local
					// arrival timestamp.
					++num_consecutive_reordered_packets_;
					if (num_consecutive_reordered_packets_ >= kReorderedResetThreshold) {
						RTC_LOG(LS_WARNING)
							<< "Packets between send burst arrived out of order, resetting."
							<< " arrival_time_delta" << arrival_time_delta->ms()
							<< " send time delta " << send_time_delta->ms();
						Reset();
					}
					return false;
				}
				else {
					num_consecutive_reordered_packets_ = 0;
				}
				*packet_size_delta = static_cast<int>(current_timestamp_group_.size) -
					static_cast<int>(prev_timestamp_group_.size);
				calculated_deltas = true;
			}
			prev_timestamp_group_ = current_timestamp_group_;
			// The new timestamp is now the current frame.
			current_timestamp_group_.first_send_time = send_time;
			current_timestamp_group_.send_time = send_time;
			current_timestamp_group_.first_arrival = arrival_time;
			current_timestamp_group_.size = 0;
		}
		else {
			current_timestamp_group_.send_time =
				std::max(current_timestamp_group_.send_time, send_time);
		}
		// Accumulate the frame size.
		// 统计分组的包大小
		current_timestamp_group_.size += packet_size;
		//更新每一个包达到时间
		current_timestamp_group_.complete_time = arrival_time;
		// 更新系统时间
		current_timestamp_group_.last_system_time = system_time;

		return calculated_deltas;
		//return false;
	}
	bool InterArrivalDelta::NewTimestampGroup(
		webrtc::Timestamp arrival_time, webrtc::Timestamp send_time) const
	{
		//突发的包分到一组 
		//  1. 两个包发送时间相同
		//  传播延迟差
		if (current_timestamp_group_.IsFirstPacket()) {
			return false;
		}
		else if (BelongsToBurst(arrival_time, send_time)) // 突发的包
		{
			return false;
		}
		else {
			return send_time - current_timestamp_group_.first_send_time >
				send_time_group_length_;
		}
	}
	bool InterArrivalDelta::BelongsToBurst(webrtc::Timestamp arrival_time, webrtc::Timestamp send_time) const
	{
		RTC_DCHECK(current_timestamp_group_.complete_time.IsFinite());
		webrtc::TimeDelta arrival_time_delta =
			arrival_time - current_timestamp_group_.complete_time;
		// 发送时间
		webrtc::TimeDelta send_time_delta = send_time - current_timestamp_group_.send_time;
		// 发送时间相同的包
		if (send_time_delta.IsZero())
		{
			return true;
		}
		 // 计算传播延迟
		webrtc::TimeDelta propagation_delta = arrival_time_delta - send_time_delta;
		if (propagation_delta < webrtc::TimeDelta::Zero() &&
			arrival_time_delta <= kBurstDeltaThreshold &&
			arrival_time - current_timestamp_group_.first_arrival < kMaxBurstDuration)
		{
			return true;
		}
		//return false;
		return false;
	}
	void InterArrivalDelta::Reset() {
		num_consecutive_reordered_packets_ = 0;
		current_timestamp_group_ = SendTimeGroup();
		prev_timestamp_group_ = SendTimeGroup();
	}
}
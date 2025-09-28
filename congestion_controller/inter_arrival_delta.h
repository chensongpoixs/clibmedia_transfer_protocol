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


				   计算两个组包延迟

 ******************************************************************************/


#ifndef _C_INTER_ARRIVAL_DELTA_H_
#define _C_INTER_ARRIVAL_DELTA_H_
#include "libmedia_transfer_protocol/media_config.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "libmedia_transfer_protocol/network_controller.h"
namespace libmtp
{

	class InterArrivalDelta
	{
	private:
		struct SendTimeGroup
		{
			SendTimeGroup()
				: size(0)
			, first_send_time(webrtc::Timestamp::MinusInfinity()) 
				, send_time(webrtc::Timestamp::MinusInfinity())
				, first_arrival(webrtc::Timestamp::MinusInfinity())
				, complete_time(webrtc::Timestamp::MinusInfinity())
				, last_system_time(webrtc::Timestamp::MinusInfinity()) 
			{}
			bool IsFirstPacket() const { return complete_time.IsInfinite(); }
			size_t   size; //数据包总和大小 分组中所有包的总和的字节数
			webrtc::Timestamp  first_send_time; // 分组第一个发送包时间
			webrtc::Timestamp send_time; // 分组中最后一个包发送时间
			webrtc::Timestamp first_arrival; // 分组中第一个达到接受端的包的时间
			webrtc::Timestamp complete_time; //  分组中最后一个达到接受端的包的时间
			webrtc::Timestamp  last_system_time; // 记录最新的系统的时间
		};
	public:
		static constexpr int kReorderedResetThreshold = 3;
		static constexpr webrtc::TimeDelta kArrivalTimeOffsetThreshold =
			webrtc::TimeDelta::Seconds(3);
	public:

		explicit InterArrivalDelta(webrtc::TimeDelta send_time_group_length);
		virtual ~InterArrivalDelta();

	public:

		// 得到 两个组包发送时间差值 send_time_delta
		 //     两个组包接受时间差差值 arrival_time_delta
		//      两个组包数据的差值 : packet_size_delta
		bool ComputeDeltas(webrtc::Timestamp send_time,
			webrtc::Timestamp arrival_time,
			webrtc::Timestamp system_time,
			size_t packet_size,
			webrtc::TimeDelta* send_time_delta,
			webrtc::TimeDelta* arrival_time_delta,
			int* packet_size_delta);

	private:
		// 是否创建新分组信息
		bool NewTimestampGroup(webrtc::Timestamp arrival_time, webrtc::Timestamp send_time) const;
		// 是否突发的包
		bool BelongsToBurst(webrtc::Timestamp arrival_time, webrtc::Timestamp send_time) const;

		
		void Reset();
	private:
		const webrtc::TimeDelta  send_time_group_length_; // 组包的长度

		SendTimeGroup current_timestamp_group_;
		SendTimeGroup prev_timestamp_group_;
		int num_consecutive_reordered_packets_;

	};
}

#endif // 
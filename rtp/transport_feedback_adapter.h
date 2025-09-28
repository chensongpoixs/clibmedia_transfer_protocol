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


#ifndef _C_TRANSPORT_FEEDBACK_ADAPTER_H_
#define _C_TRANSPORT_FEEDBACK_ADAPTER_H_

#include <cstddef>

#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/network_controller.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/transport_feedback.h"
#include "libmedia_transfer_protocol/congestion_controller/goog_cc_network_controller.h"
#include "libice/network_types.h"
#include "rtc_base/network_route.h"
#include <map>
#include "modules/include/module_common_types_public.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet.h"
#include "rtc_base/network/sent_packet.h"
namespace libmedia_transfer_protocol {

	struct PacketFeedback {
		PacketFeedback() = default;
		// Time corresponding to when this object was created.
		webrtc::Timestamp creation_time = webrtc::Timestamp::MinusInfinity();
		libice::SentPacket sent;
		// Time corresponding to when the packet was received. Timestamped with the
		// receiver's clock. For unreceived packet, Timestamp::PlusInfinity() is
		// used.
		webrtc::Timestamp receive_time = webrtc::Timestamp::PlusInfinity();

		// The network route that this packet is associated with.
		rtc::NetworkRoute network_route;
	};
	class TransportFeedbackAdapter
	{
	public:
		TransportFeedbackAdapter();
		virtual ~TransportFeedbackAdapter();

	public:
		
		// 发送端 记录数据包数据结构
		void AddPacket(webrtc::Timestamp creation_time, 
			size_t overhead_bytes, const libmedia_transfer_protocol::RtpPacketSendInfo& send_info);



		absl::optional<libice::SentPacket>  ProcessSentPacket(const rtc::SentPacket& send_packet);
		absl::optional<libice::TransportPacketsFeedback> ProcessTransportFeedback(
			const rtcp::TransportFeedback& feedback, webrtc::Timestamp feedback_time);


		std::vector<libice::PacketResult> ProcessTransportFeedbackInner(const rtcp::TransportFeedback& feedback, webrtc::Timestamp feedback_time);
	private:
		// 当前偏移的时间
		webrtc::Timestamp  current_offset_ = webrtc::Timestamp::MinusInfinity();
		webrtc::TimeDelta  last_timestamp_ = webrtc::TimeDelta::MinusInfinity();


		std::map<int64_t, PacketFeedback>    history_;
		webrtc::SequenceNumberUnwrapper seq_num_unwrapper_;

		webrtc::Timestamp      last_send_time_ = webrtc::Timestamp::MinusInfinity();


		int64_t     last_ack_seq_num_ = -1;
	};
}


#endif // 
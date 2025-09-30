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

#include "libmedia_transfer_protocol/congestion_controller/goog_cc_network_controller.h"

#include "rtc_base/logging.h"
namespace libmtp
{
	GoogCcNetworkController::GoogCcNetworkController()
		: delay_based_bwe_( std::make_unique< DelayBasedBwe>())
		, acknowledge_bitrate_estimator_(AcknowledgedBitrateEstimatorInterface::Create())
	{
		//������ʼ����
		delay_based_bwe_->SetStartBitrate(webrtc::DataRate::KilobitsPerSec(300));
	}
	GoogCcNetworkController:: ~GoogCcNetworkController()
	{

	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnTransportPacketsFeedback(
		const libice::TransportPacketsFeedback & report)
	{
		if (report.packet_feedbacks.empty())
		{
			return libice::NetworkControlUpdate();
		}


		//absl::optional<webrtc::DataRate> acked_bitrate = webrtc::DataRate::KilobitsPerSec(50000);
		//���õ�ǰ���������Ĵ�С  �Է�ȷ���յ�������
		acknowledge_bitrate_estimator_->IncomingPacketFeedbackVector(report.SortedByReceiveTime());
		
		

		absl::optional<webrtc::DataRate> acked_bitrate = acknowledge_bitrate_estimator_->bitrate();
		
		if (acked_bitrate.has_value())
		{
			RTC_LOG(LS_INFO) << "ack_bitrate : " << webrtc::ToString(*acked_bitrate);
		}
		//acked_bitrate.emplace(webrtc::DataRate::KilobitsPerSec(50000));
			absl::optional<webrtc::DataRate> probe_bitrate;
			absl::optional<libice::NetworkStateEstimate> network_estimate;
		DelayBasedBwe::Result result = delay_based_bwe_->IncomingPacketFeedbackVector(
			report, acked_bitrate, probe_bitrate, network_estimate, false);
		//RTC_LOG(LS_INFO) << "delay bwe:" << result.ToString();
		return libice::NetworkControlUpdate();
	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnRttUpdate(int64_t rtt_ms)
	{
		delay_based_bwe_->OnRttUpdate(rtt_ms);
		return libice::NetworkControlUpdate();
	}
}
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

													1、基于延迟的带宽估计（DelayBasedBwe）									

													2、基于掉包的带宽估计(SendSideBandwidthEstimation)

	GoogCcNetworkController  依赖五大模块 ===>         3、吞吐量估计器(AcknowledgedBitrateEstimator)

													4、应用限制区域检测器(AlrDetector)

													5、码率快速探测Probe （ProbeController、ProbeBitrateEstimator）




 ******************************************************************************/


#ifndef _C_GOOG_CC_NETWORK_CONTROLLER_H_
#define _C_GOOG_CC_NETWORK_CONTROLLER_H_
#include "libmedia_transfer_protocol/media_config.h"
#include "libmedia_transfer_protocol/congestion_controller/delay_based_bwe.h"
#include "libmedia_transfer_protocol/network_controller.h"
#include "libmedia_transfer_protocol/congestion_controller/acknowledged_bitrate_estimator.h"
#include "libmedia_transfer_protocol/congestion_controller/loss_based_bandwidth_estimation.h"
#include "libmedia_transfer_protocol/congestion_controller/send_side_bandwidth_estimation.h"
namespace libmtp
{
	class GoogCcNetworkController : public NetworkControllerInterface
	{
	public:
		GoogCcNetworkController(const NetworkControllerConfig& config);
		virtual ~GoogCcNetworkController() override;
		virtual libice::NetworkControlUpdate OnTransportPacketsFeedback(
			const libice::TransportPacketsFeedback& msg) override;

		virtual  libice::NetworkControlUpdate OnRttUpdate(int64_t rtt_ms, webrtc::Timestamp at_time) override;


		virtual libice::NetworkControlUpdate OnTransportLossReport(
			libice::TransportLossReport)  override;

		// 网络连接成功后调用  start min max bitrate
		virtual libice::NetworkControlUpdate OnTargetRateConstraints(
			libice::TargetRateConstraints) override;
	private:
		void MaybeTriggerOnNetworkChanged(libice::NetworkControlUpdate* update,
			webrtc::Timestamp at_time);

		libice::PacerConfig GetPacingRates(webrtc::Timestamp at_time) const;
	private:

		std::unique_ptr<DelayBasedBwe>  delay_based_bwe_;

		std::unique_ptr< AcknowledgedBitrateEstimatorInterface>  acknowledge_bitrate_estimator_;
		std::unique_ptr< SendSideBandwidthEstimation>    bandwidth_estimation_;


		webrtc::DataRate                                     last_loss_based_bitrate_;

		uint8_t last_estimated_fraction_loss_ = 0;
		webrtc::TimeDelta last_estimated_round_trip_time_ = webrtc::TimeDelta::PlusInfinity();


		webrtc::TimeDelta  last_estimated_rtt_ = webrtc::TimeDelta::PlusInfinity();
		webrtc::DataRate last_loss_based_target_rate_;
		webrtc::DataRate last_pushback_target_rate_;
		webrtc::DataRate last_stable_target_rate_;

		double pacing_factor_;
		webrtc::DataRate min_total_allocated_bitrate_;
		webrtc::DataRate max_padding_rate_;
		webrtc::DataRate max_total_allocated_bitrate_;
	};
}

#endif // _C_GOOG_CC_NETWORK_CONTROLLER_H_
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
	GoogCcNetworkController::GoogCcNetworkController(const NetworkControllerConfig& config)
		: delay_based_bwe_( std::make_unique< DelayBasedBwe>())
		, acknowledge_bitrate_estimator_(AcknowledgedBitrateEstimatorInterface::Create())
		, bandwidth_estimation_(std::make_unique<SendSideBandwidthEstimation>())
		, last_loss_based_bitrate_(webrtc::DataRate::Zero())
	{
		//设置起始码流
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
		//设置当前的吞吐量的大小  对方确认收到的数据
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
		//基于延迟的带宽估计值更新了， 需要设置到基于掉包的带宽估计模块
		libice::NetworkControlUpdate update;
		if (result.updated)
		{
			bandwidth_estimation_->UpdateDelayBasedEstimate(report.feedback_time, result.target_bitrate);
		
			MaybeTriggerOnNetworkChanged(&update, report.feedback_time);
		}
		//RTC_LOG(LS_INFO) << "delay bwe:" << result.ToString();
		return update;
	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnRttUpdate(int64_t rtt_ms, webrtc::Timestamp at_time)
	{
		bandwidth_estimation_->UpdateRtt(webrtc::TimeDelta::Millis(rtt_ms), at_time);
		delay_based_bwe_->OnRttUpdate(rtt_ms);
		return libice::NetworkControlUpdate();
	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnTransportLossReport(
		libice::TransportLossReport msg)
	{
		int64_t total_packets_delta =
			msg.packets_received_delta + msg.packets_lost_delta;
		bandwidth_estimation_->UpdatePacketsLost(
			msg.packets_lost_delta, total_packets_delta, msg.receive_time);


		webrtc::DataRate lost_rate = bandwidth_estimation_->target_rate();
		RTC_LOG(LS_INFO) << "lost rate: " << webrtc::ToString(lost_rate);
		return libice::NetworkControlUpdate();
	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnTargetRateConstraints(
		libice::TargetRateConstraints constraints)
	{
		webrtc::DataRate starting_rate =  constraints.starting_rate.has_value() ? constraints.starting_rate.value(): webrtc::DataRate::Zero();
		webrtc::DataRate min_data_rate = constraints.min_data_rate.has_value() ? constraints.min_data_rate.value() : webrtc::DataRate::Zero();
		webrtc::DataRate max_data_rate = constraints.max_data_rate.has_value() ? constraints.max_data_rate.value() : webrtc::DataRate::Zero();
		 

		bandwidth_estimation_->SetBitrates( starting_rate,  min_data_rate,
			 max_data_rate, constraints.at_time);
		return libice::NetworkControlUpdate();
	}
	void GoogCcNetworkController::MaybeTriggerOnNetworkChanged(
		libice::NetworkControlUpdate * update, webrtc::Timestamp at_time)
	{

		uint8_t fraction_loss = bandwidth_estimation_->fraction_loss();
		webrtc::TimeDelta round_trip_time = bandwidth_estimation_->round_trip_time();
		webrtc::DataRate loss_based_target_rate = bandwidth_estimation_->target_rate();
		webrtc::DataRate pushback_target_rate = loss_based_target_rate;
	
		if (last_loss_based_bitrate_ != loss_based_target_rate||
			last_estimated_fraction_loss_ != fraction_loss||
			last_estimated_round_trip_time_ != round_trip_time)
		{
			last_estimated_fraction_loss_ = fraction_loss;
			last_loss_based_bitrate_ = loss_based_target_rate;
			last_estimated_round_trip_time_ = round_trip_time;

			libice::TargetTransferRate  targettransferrate;
			targettransferrate.at_time = at_time;
			targettransferrate.target_rate = loss_based_target_rate;
			
			update->target_rate = targettransferrate;
			RTC_LOG(LS_INFO) << "update bwe ==> target_rate: " <<webrtc::ToString(loss_based_target_rate);
		}
		//BWE_TEST_LOGGING_PLOT(1, "fraction_loss_%", at_time.ms(),
		//	(fraction_loss * 100) / 256);
		//BWE_TEST_LOGGING_PLOT(1, "rtt_ms", at_time.ms(), round_trip_time.ms());
		//BWE_TEST_LOGGING_PLOT(1, "Target_bitrate_kbps", at_time.ms(),
		//	loss_based_target_rate.kbps());

		//double cwnd_reduce_ratio = 0.0;
		//if (congestion_window_pushback_controller_) {
		//	int64_t pushback_rate =
		//		congestion_window_pushback_controller_->UpdateTargetBitrate(
		//			loss_based_target_rate.bps());
		//	pushback_rate = std::max<int64_t>(bandwidth_estimation_->GetMinBitrate(),
		//		pushback_rate);
		//	pushback_target_rate = DataRate::BitsPerSec(pushback_rate);
		//	if (rate_control_settings_.UseCongestionWindowDropFrameOnly()) {
		//		cwnd_reduce_ratio = static_cast<double>(loss_based_target_rate.bps() -
		//			pushback_target_rate.bps()) /
		//			loss_based_target_rate.bps();
		//	}
		//}
		//webrtc::DataRate stable_target_rate =
		//	bandwidth_estimation_->GetEstimatedLinkCapacity();
		//if (loss_based_stable_rate_) 
		//{
		//	stable_target_rate = std::min(stable_target_rate, loss_based_target_rate);
		//}
		//else {
		//	stable_target_rate = std::min(stable_target_rate, pushback_target_rate);
		//}
		//
		//if ((loss_based_target_rate != last_loss_based_target_rate_) ||
		//	(fraction_loss != last_estimated_fraction_loss_) ||
		//	(round_trip_time != last_estimated_round_trip_time_) ||
		//	(pushback_target_rate != last_pushback_target_rate_) ||
		//	(stable_target_rate != last_stable_target_rate_)) {
		//	last_loss_based_target_rate_ = loss_based_target_rate;
		//	last_pushback_target_rate_ = pushback_target_rate;
		//	last_estimated_fraction_loss_ = fraction_loss;
		//	last_estimated_round_trip_time_ = round_trip_time;
		//	last_stable_target_rate_ = stable_target_rate;
		//
		//	alr_detector_->SetEstimatedBitrate(loss_based_target_rate.bps());
		//
		//	TimeDelta bwe_period = delay_based_bwe_->GetExpectedBwePeriod();
		//
		//	TargetTransferRate target_rate_msg;
		//	target_rate_msg.at_time = at_time;
		//	if (rate_control_settings_.UseCongestionWindowDropFrameOnly()) {
		//		target_rate_msg.target_rate = loss_based_target_rate;
		//		target_rate_msg.cwnd_reduce_ratio = cwnd_reduce_ratio;
		//	}
		//	else {
		//		target_rate_msg.target_rate = pushback_target_rate;
		//	}
		//	target_rate_msg.stable_target_rate = stable_target_rate;
		//	target_rate_msg.network_estimate.at_time = at_time;
		//	target_rate_msg.network_estimate.round_trip_time = round_trip_time;
		//	target_rate_msg.network_estimate.loss_rate_ratio = fraction_loss / 255.0f;
		//	target_rate_msg.network_estimate.bwe_period = bwe_period;
		//
		//	update->target_rate = target_rate_msg;
		//
		//	auto probes = probe_controller_->SetEstimatedBitrate(
		//		loss_based_target_rate.bps(), at_time.ms());
		//	update->probe_cluster_configs.insert(update->probe_cluster_configs.end(),
		//		probes.begin(), probes.end());
		//	update->pacer_config = GetPacingRates(at_time);
		//
		//	RTC_LOG(LS_VERBOSE) << "bwe " << at_time.ms() << " pushback_target_bps="
		//		<< last_pushback_target_rate_.bps()
		//		<< " estimate_bps=" << loss_based_target_rate.bps();
		//}
	}
}
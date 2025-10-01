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
	// From RTCPSender video report interval.
	constexpr webrtc   ::TimeDelta kLossUpdateInterval = webrtc::TimeDelta::Millis(1000);

	// Pacing-rate relative to our target send rate.
	// Multiplicative factor that is applied to the target bitrate to calculate
	// the number of bytes that can be transmitted per interval.
	// Increasing this factor will result in lower delays in cases of bitrate
	// overshoots from the encoder.
	constexpr float kDefaultPaceMultiplier = 2.5f;

	// If the probe result is far below the current throughput estimate
	// it's unlikely that the probe is accurate, so we don't want to drop too far.
	// However, if we actually are overusing, we want to drop to something slightly
	// below the current throughput estimate to drain the network queues.
	constexpr double kProbeDropThroughputFraction = 0.85;
	GoogCcNetworkController::GoogCcNetworkController(const NetworkControllerConfig& config)
		: delay_based_bwe_( std::make_unique< DelayBasedBwe>())
		, acknowledge_bitrate_estimator_(AcknowledgedBitrateEstimatorInterface::Create())
		, bandwidth_estimation_(std::make_unique<SendSideBandwidthEstimation>())
		, last_loss_based_bitrate_(webrtc::DataRate::Zero())
		, last_loss_based_target_rate_(webrtc::DataRate::Zero())
		, last_pushback_target_rate_(webrtc::DataRate::Zero())
		, last_stable_target_rate_(webrtc::DataRate::Zero())
		, pacing_factor_(kDefaultPaceMultiplier) 
		, min_total_allocated_bitrate_(webrtc::DataRate::Zero())
		, max_padding_rate_(webrtc::DataRate::Zero())
		, max_total_allocated_bitrate_(webrtc::DataRate::Zero())
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
		libice::NetworkControlUpdate update;
		MaybeTriggerOnNetworkChanged(&update, constraints.at_time);
		return update;
	}
	void GoogCcNetworkController::MaybeTriggerOnNetworkChanged(
		libice::NetworkControlUpdate * update, webrtc::Timestamp at_time)
	{

		 uint8_t fraction_loss = bandwidth_estimation_->fraction_loss();
  webrtc::TimeDelta round_trip_time = bandwidth_estimation_->round_trip_time();
  webrtc::DataRate loss_based_target_rate = bandwidth_estimation_->target_rate();
  //webrtc::DataRate pushback_target_rate = loss_based_target_rate;

 

  //double cwnd_reduce_ratio = 0.0;
 
  //webrtc::DataRate stable_target_rate =
  //    bandwidth_estimation_->GetEstimatedLinkCapacity();
  //  stable_target_rate = std::min(stable_target_rate, loss_based_target_rate);
  

  if ((loss_based_target_rate != last_loss_based_bitrate_) ||
      (fraction_loss != last_estimated_fraction_loss_) ||
      (round_trip_time != last_estimated_rtt_) //||
     // (pushback_target_rate != last_pushback_target_rate_) ||
     // (stable_target_rate != last_stable_target_rate_)
	  ) 
  {
	  last_loss_based_bitrate_ = loss_based_target_rate;
     
    last_estimated_fraction_loss_ = fraction_loss;
	last_estimated_rtt_ = round_trip_time;
    

    //alr_detector_->SetEstimatedBitrate(loss_based_target_rate.bps());

    //webrtc::TimeDelta bwe_period = delay_based_bwe_->GetExpectedBwePeriod();

   libice:: TargetTransferRate target_rate_msg;
    target_rate_msg.at_time = at_time;
    //if (rate_control_settings_.UseCongestionWindowDropFrameOnly()) {
    //  target_rate_msg.target_rate = loss_based_target_rate;
    //  target_rate_msg.cwnd_reduce_ratio = cwnd_reduce_ratio;
    //} else {
      target_rate_msg.target_rate = loss_based_target_rate;
    //}
 //   target_rate_msg.stable_target_rate = stable_target_rate;
   //target_rate_msg.network_estimate.at_time = at_time;
   //target_rate_msg.network_estimate.round_trip_time = round_trip_time;
   //target_rate_msg.network_estimate.loss_rate_ratio = fraction_loss / 255.0f;
   //target_rate_msg.network_estimate.bwe_period = bwe_period;

    update->target_rate = target_rate_msg;

    //auto probes = probe_controller_->SetEstimatedBitrate(
       // loss_based_target_rate.bps(), at_time.ms());
  //  update->probe_cluster_configs.insert(update->probe_cluster_configs.end(),
                   //                      probes.begin(), probes.end());
    update->pacer_config = GetPacingRates(at_time);

    RTC_LOG(LS_INFO) << "bwe " /*<< at_time.ms()*/ << " pushback_target_bps="
                        << last_loss_based_bitrate_.bps();
  }
	}
	libice::PacerConfig GoogCcNetworkController::GetPacingRates(webrtc::Timestamp at_time) const
	{
		// Pacing rate is based on target rate before congestion window pushback,
  // because we don't want to build queues in the pacer when pushback occurs.
		// 编码器  输出码流不是太正确和fec， 所以带宽需要适当提高
		webrtc::DataRate pacing_rate =
			std::max(min_total_allocated_bitrate_, last_loss_based_bitrate_) *
			pacing_factor_;
		webrtc::DataRate padding_rate =
			std::min(max_padding_rate_, last_pushback_target_rate_);
		libice::PacerConfig msg;
		msg.at_time = at_time;
		msg.time_window = webrtc::TimeDelta::Seconds(1);
		msg.data_window = pacing_rate * msg.time_window;
		msg.pad_window = padding_rate * msg.time_window;
		return msg;
	}

	libice::NetworkControlUpdate GoogCcNetworkController::OnProcessInterval(
		libice::ProcessInterval msg)
	{
		bandwidth_estimation_->UpdateEstimate(msg.at_time);
		libice::NetworkControlUpdate update;
		MaybeTriggerOnNetworkChanged(&update, msg.at_time);
		return update;
	}
}
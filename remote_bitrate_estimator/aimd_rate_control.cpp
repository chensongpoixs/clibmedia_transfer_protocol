
/*****************************************************************************
				  Author: chensong
				  date:  2025-09-30


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


				  AIMD 码流控制

				  增加码率：  加法
				  减低码率：  减法倍速 子速

******************************************************************************/



#include "libmedia_transfer_protocol/remote_bitrate_estimator/aimd_rate_control.h"

#include <inttypes.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "absl/strings/match.h"
#include "libice/network_types.h"
#include "api/units/data_rate.h"
#include "libmedia_transfer_protocol/remote_bitrate_estimator/bwe_defines.h"
#include "libmedia_transfer_protocol/remote_bitrate_estimator/overuse_detector.h"
#include "rtc_base/checks.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/logging.h"
#include "rtc_base/numerics/safe_minmax.h"

namespace libmedia_transfer_protocol {
namespace {

constexpr webrtc::TimeDelta kDefaultRtt = webrtc::TimeDelta::Millis(200);
constexpr double kDefaultBackoffFactor = 0.85;

constexpr char kBweBackOffFactorExperiment[] = "WebRTC-BweBackOffFactor";

//bool IsEnabled(const WebRtcKeyValueConfig& field_trials,
//               absl::string_view key) {
//  return absl::StartsWith(field_trials.Lookup(key), "Enabled");
//}
//
//bool IsNotDisabled(const WebRtcKeyValueConfig& field_trials,
//                   absl::string_view key) {
//  return !absl::StartsWith(field_trials.Lookup(key), "Disabled");
//}

double ReadBackoffFactor(/*const WebRtcKeyValueConfig& key_value_config*/) {
  /*std::string experiment_string =
      key_value_config.Lookup(kBweBackOffFactorExperiment);
  double backoff_factor;
  int parsed_values =
      sscanf(experiment_string.c_str(), "Enabled-%lf", &backoff_factor);
  if (parsed_values == 1) {
    if (backoff_factor >= 1.0) {
      RTC_LOG(WARNING) << "Back-off factor must be less than 1.";
    } else if (backoff_factor <= 0.0) {
      RTC_LOG(WARNING) << "Back-off factor must be greater than 0.";
    } else {
      return backoff_factor;
    }
  }*/
  RTC_LOG(LS_WARNING) << "Failed to parse parameters for AimdRateControl "
                         "experiment from field trial string. Using default.";
  return kDefaultBackoffFactor;
}

}  // namespace

AimdRateControl::AimdRateControl(/*const WebRtcKeyValueConfig* key_value_config*/)
    : AimdRateControl(/*key_value_config,*/ /* send_side =*/false) {}

AimdRateControl::AimdRateControl(/*const WebRtcKeyValueConfig* key_value_config,*/
                                 bool send_side)
    : min_configured_bitrate_(congestion_controller::GetMinBitrate()),
      max_configured_bitrate_(webrtc::DataRate::KilobitsPerSec(30000)),
      current_bitrate_(max_configured_bitrate_),
      latest_estimated_throughput_(current_bitrate_),
    //  link_capacity_(),
      rate_control_state_(RateControlState::kRcHold),
      time_last_bitrate_change_(webrtc::Timestamp::MinusInfinity()),
      time_last_bitrate_decrease_(webrtc::Timestamp::MinusInfinity()),
      time_first_throughput_estimate_(webrtc::Timestamp::MinusInfinity()),
      bitrate_is_initialized_(false),
      beta_(/*IsEnabled(*key_value_config, kBweBackOffFactorExperiment)
                ? ReadBackoffFactor(*key_value_config)
                : */kDefaultBackoffFactor),
      in_alr_(false),
      rtt_(kDefaultRtt),
      send_side_(send_side),
      in_experiment_( false/*!AdaptiveThresholdExperimentIsDisabled(*key_value_config)*/),
      no_bitrate_increase_in_alr_( false
          /*IsEnabled(*key_value_config,
                    "WebRTC-DontIncreaseDelayBasedBweInAlr")*/),
      estimate_bounded_backoff_( false
          /*IsNotDisabled(*key_value_config,
                        "WebRTC-Bwe-EstimateBoundedBackoff")*/),
      estimate_bounded_increase_( false
         /* IsNotDisabled(*key_value_config,
                        "WebRTC-Bwe-EstimateBoundedIncrease")*/)
      //initial_backoff_interval_("initial_backoff_interval"),
     // link_capacity_fix_("link_capacity_fix") 
{
  // E.g
  // WebRTC-BweAimdRateControlConfig/initial_backoff_interval:100ms/
  //ParseFieldTrial({&initial_backoff_interval_, &link_capacity_fix_},
  //                key_value_config->Lookup("WebRTC-BweAimdRateControlConfig"));
  //if (initial_backoff_interval_) {
  //  RTC_LOG(LS_INFO) << "Using aimd rate control with initial back-off interval"
  //                      " "
  //                   << ToString(*initial_backoff_interval_) << ".";
  //}
  RTC_LOG(LS_INFO) << "Using aimd rate control with back off factor " << beta_;
}

AimdRateControl::~AimdRateControl() {}

void AimdRateControl::SetStartBitrate(webrtc::DataRate start_bitrate) {
  current_bitrate_ = start_bitrate;
  latest_estimated_throughput_ = current_bitrate_;
  bitrate_is_initialized_ = true;
}

void AimdRateControl::SetMinBitrate(webrtc::DataRate min_bitrate) {
  min_configured_bitrate_ = min_bitrate;
  current_bitrate_ = std::max(min_bitrate, current_bitrate_);
}

bool AimdRateControl::ValidEstimate() const {
  return bitrate_is_initialized_;
}

webrtc::TimeDelta AimdRateControl::GetFeedbackInterval() const {
  // Estimate how often we can send RTCP if we allocate up to 5% of bandwidth
  // to feedback.
  const webrtc::DataSize kRtcpSize = webrtc::DataSize::Bytes(80);
  const webrtc::DataRate rtcp_bitrate = current_bitrate_ * 0.05;
  const webrtc::TimeDelta interval = kRtcpSize / rtcp_bitrate;
  const webrtc::TimeDelta kMinFeedbackInterval = webrtc::TimeDelta::Millis(200);
  const webrtc::TimeDelta kMaxFeedbackInterval = webrtc::TimeDelta::Millis(1000);
  return interval.Clamped(kMinFeedbackInterval, kMaxFeedbackInterval);
}

bool AimdRateControl::TimeToReduceFurther(webrtc::Timestamp at_time,
	webrtc::DataRate estimated_throughput) const {
  const webrtc::TimeDelta bitrate_reduction_interval =
      rtt_.Clamped(webrtc::TimeDelta::Millis(10), webrtc::TimeDelta::Millis(200));
  // 上一次是否满足200ms 
  if (at_time - time_last_bitrate_change_ >= bitrate_reduction_interval) 
  {
    return true;
  }
  //当前码流中一个必须要大于吞吐量， 避免毛利率降低过低
  if (ValidEstimate()) {
    // TODO(terelius/holmer): Investigate consequences of increasing
    // the threshold to 0.95 * LatestEstimate().
    const webrtc::DataRate threshold = 0.5 * LatestEstimate();
    return estimated_throughput < threshold;
  }
  return false;
}

bool AimdRateControl::InitialTimeToReduceFurther(webrtc::Timestamp at_time) const {
  //if (!initial_backoff_interval_) {
	//码流模块是否有效的 然后在判断当前码流 一半的码流
    return ValidEstimate() &&
           TimeToReduceFurther(at_time,
                               LatestEstimate() / 2 - webrtc::DataRate::BitsPerSec(1));
  //}
  //// TODO(terelius): We could use the RTT (clamped to suitable limits) instead
  //// of a fixed bitrate_reduction_interval.
  //if (time_last_bitrate_decrease_.IsInfinite() ||
  //    at_time - time_last_bitrate_decrease_ >= *initial_backoff_interval_) {
  //  return true;
  //}
  return false;
}

webrtc::DataRate AimdRateControl::LatestEstimate() const {
  return current_bitrate_;
}

void AimdRateControl::SetRtt(webrtc::TimeDelta rtt) {
  rtt_ = rtt;
}

webrtc::DataRate AimdRateControl::Update(const RateControlInput* input,
	webrtc::Timestamp at_time) {
  RTC_CHECK(input);

  // Set the initial bit rate value to what we're receiving the first half
  // second.
  // TODO(bugs.webrtc.org/9379): The comment above doesn't match to the code.
  if (!bitrate_is_initialized_) {
    const webrtc::TimeDelta kInitializationTime = webrtc::TimeDelta::Seconds(5);
    RTC_DCHECK_LE(kBitrateWindowMs, kInitializationTime.ms());
    if (time_first_throughput_estimate_.IsInfinite()) {
      if (input->estimated_throughput)
        time_first_throughput_estimate_ = at_time;
    } else if (at_time - time_first_throughput_estimate_ >
                   kInitializationTime &&
               input->estimated_throughput) {
      current_bitrate_ = *input->estimated_throughput;
      bitrate_is_initialized_ = true;
    }
  }

  ChangeBitrate(*input, at_time);
  return current_bitrate_;
}

void AimdRateControl::SetInApplicationLimitedRegion(bool in_alr) {
  in_alr_ = in_alr;
}

void AimdRateControl::SetEstimate(webrtc::DataRate bitrate, webrtc::Timestamp at_time) {
  bitrate_is_initialized_ = true;
  webrtc::DataRate prev_bitrate = current_bitrate_;
  current_bitrate_ = ClampBitrate(bitrate);
  time_last_bitrate_change_ = at_time;
  if (current_bitrate_ < prev_bitrate) {
    time_last_bitrate_decrease_ = at_time;
  }
}

void AimdRateControl::SetNetworkStateEstimate(
    const absl::optional<libice::NetworkStateEstimate>& estimate) {
  network_estimate_ = estimate;
}

double AimdRateControl::GetNearMaxIncreaseRateBpsPerSecond() const {
  RTC_DCHECK(!current_bitrate_.IsZero());
  const webrtc::TimeDelta kFrameInterval = webrtc::TimeDelta::Seconds(1) / 30;
  webrtc::DataSize frame_size = current_bitrate_ * kFrameInterval;
  const webrtc::DataSize kPacketSize = webrtc::DataSize::Bytes(1200);
  double packets_per_frame = std::ceil(frame_size / kPacketSize);
  webrtc::DataSize avg_packet_size = frame_size / packets_per_frame;

  // Approximate the over-use estimator delay to 100 ms.
  webrtc::TimeDelta response_time = rtt_ + webrtc::TimeDelta::Millis(100);
  if (in_experiment_)
    response_time = response_time * 2;
  double increase_rate_bps_per_second =
      (avg_packet_size / response_time).bps<double>();
  double kMinIncreaseRateBpsPerSecond = 4000;
  return std::max(kMinIncreaseRateBpsPerSecond, increase_rate_bps_per_second);
}

webrtc::TimeDelta AimdRateControl::GetExpectedBandwidthPeriod() const {
  const webrtc::TimeDelta kMinPeriod = webrtc::TimeDelta::Seconds(2);
  const webrtc::TimeDelta kDefaultPeriod = webrtc::TimeDelta::Seconds(3);
  const webrtc::TimeDelta kMaxPeriod = webrtc::TimeDelta::Seconds(50);

  double increase_rate_bps_per_second = GetNearMaxIncreaseRateBpsPerSecond();
  if (!last_decrease_)
    return kDefaultPeriod;
  double time_to_recover_decrease_seconds =
      last_decrease_->bps() / increase_rate_bps_per_second;
  webrtc::TimeDelta period = webrtc::TimeDelta::Seconds(time_to_recover_decrease_seconds);
  return period.Clamped(kMinPeriod, kMaxPeriod);
}

void AimdRateControl::ChangeBitrate(const RateControlInput& input,
	webrtc::Timestamp at_time) {
  absl::optional<webrtc::DataRate> new_bitrate;
  webrtc::DataRate estimated_throughput =
      input.estimated_throughput.value_or(latest_estimated_throughput_);
  if (input.estimated_throughput)
    latest_estimated_throughput_ = *input.estimated_throughput;

  // An over-use should always trigger us to reduce the bitrate, even though
  // we have not yet established our first estimate. By acting on the over-use,
  // we will end up with a valid estimate.
  if (!bitrate_is_initialized_ &&
      input.bw_state != BandwidthUsage::kBwOverusing)
    return;

  ChangeState(input, at_time);

  // We limit the new bitrate based on the troughput to avoid unlimited bitrate
  // increases. We allow a bit more lag at very low rates to not too easily get
  // stuck if the encoder produces uneven outputs.
  const webrtc::DataRate troughput_based_limit =
      1.5 * estimated_throughput + webrtc::DataRate::KilobitsPerSec(10);

  switch (rate_control_state_) {
    case RateControlState::kRcHold:
      break;

    case RateControlState::kRcIncrease:
      if (estimated_throughput > link_capacity_.UpperBound())
        link_capacity_.Reset();

      // Do not increase the delay based estimate in alr since the estimator
      // will not be able to get transport feedback necessary to detect if
      // the new estimate is correct.
      // If we have previously increased above the limit (for instance due to
      // probing), we don't allow further changes.
      if (current_bitrate_ < troughput_based_limit &&
          !(send_side_ && in_alr_ && no_bitrate_increase_in_alr_)) {
		  webrtc::DataRate increased_bitrate = webrtc::DataRate::MinusInfinity();
        if (link_capacity_.has_estimate()) {
          // The link_capacity estimate is reset if the measured throughput
          // is too far from the estimate. We can therefore assume that our
          // target rate is reasonably close to link capacity and use additive
          // increase.
			webrtc::DataRate additive_increase =
              AdditiveRateIncrease(at_time, time_last_bitrate_change_);
          increased_bitrate = current_bitrate_ + additive_increase;
        } else {
          // If we don't have an estimate of the link capacity, use faster ramp
          // up to discover the capacity.
			webrtc::DataRate multiplicative_increase = MultiplicativeRateIncrease(
              at_time, time_last_bitrate_change_, current_bitrate_);
          increased_bitrate = current_bitrate_ + multiplicative_increase;
        }
        new_bitrate = std::min(increased_bitrate, troughput_based_limit);
      }

      time_last_bitrate_change_ = at_time;
      break;

    case RateControlState::kRcDecrease: {
		webrtc::DataRate decreased_bitrate = webrtc::DataRate::PlusInfinity();

      // Set bit rate to something slightly lower than the measured throughput
      // to get rid of any self-induced delay.
      decreased_bitrate = estimated_throughput * beta_;
      if (decreased_bitrate > current_bitrate_ /*&& !link_capacity_fix_*/) {
        // TODO(terelius): The link_capacity estimate may be based on old
        // throughput measurements. Relying on them may lead to unnecessary
        // BWE drops.
        if (link_capacity_.has_estimate()) {
          decreased_bitrate = beta_ * link_capacity_.estimate();
        }
      }
      if (estimate_bounded_backoff_ && network_estimate_) {
        decreased_bitrate = std::max(
            decreased_bitrate, network_estimate_->link_capacity_lower * beta_);
      }

      // Avoid increasing the rate when over-using.
      if (decreased_bitrate < current_bitrate_) {
        new_bitrate = decreased_bitrate;
      }

      if (bitrate_is_initialized_ && estimated_throughput < current_bitrate_) {
        if (!new_bitrate.has_value()) {
          last_decrease_ = webrtc::DataRate::Zero();
        } else {
          last_decrease_ = current_bitrate_ - *new_bitrate;
        }
      }
      if (estimated_throughput < link_capacity_.LowerBound()) {
        // The current throughput is far from the estimated link capacity. Clear
        // the estimate to allow an immediate update in OnOveruseDetected.
        link_capacity_.Reset();
      }

      bitrate_is_initialized_ = true;
      link_capacity_.OnOveruseDetected(estimated_throughput);
      // Stay on hold until the pipes are cleared.
      rate_control_state_ = RateControlState::kRcHold;
      time_last_bitrate_change_ = at_time;
      time_last_bitrate_decrease_ = at_time;
      break;
    }
    default:
      RTC_NOTREACHED();
  }

  current_bitrate_ = ClampBitrate(new_bitrate.value_or(current_bitrate_));
}

webrtc::DataRate AimdRateControl::ClampBitrate(webrtc::DataRate new_bitrate) const {
  if (estimate_bounded_increase_ && network_estimate_) {
	  webrtc::DataRate upper_bound = network_estimate_->link_capacity_upper;
    new_bitrate = std::min(new_bitrate, upper_bound);
  }
  new_bitrate = std::max(new_bitrate, min_configured_bitrate_);
  return new_bitrate;
}

webrtc::DataRate AimdRateControl::MultiplicativeRateIncrease(
	webrtc::Timestamp at_time,
	webrtc::Timestamp last_time,
	webrtc::DataRate current_bitrate) const {
  double alpha = 1.08;
  if (last_time.IsFinite()) {
    auto time_since_last_update = at_time - last_time;
    alpha = pow(alpha, std::min(time_since_last_update.seconds<double>(), 1.0));
  }
  webrtc::DataRate multiplicative_increase =
      std::max(current_bitrate * (alpha - 1.0), webrtc::DataRate::BitsPerSec(1000));
  return multiplicative_increase;
}

webrtc::DataRate AimdRateControl::AdditiveRateIncrease(webrtc::Timestamp at_time,
	webrtc::Timestamp last_time) const {
  double time_period_seconds = (at_time - last_time).seconds<double>();
  double data_rate_increase_bps =
      GetNearMaxIncreaseRateBpsPerSecond() * time_period_seconds;
  return webrtc::DataRate::BitsPerSec(data_rate_increase_bps);
}

void AimdRateControl::ChangeState(const RateControlInput& input,
	webrtc::Timestamp at_time) {
  switch (input.bw_state) {
    case BandwidthUsage::kBwNormal:
      if (rate_control_state_ == RateControlState::kRcHold) {
        time_last_bitrate_change_ = at_time;
        rate_control_state_ = RateControlState::kRcIncrease;
      }
      break;
    case BandwidthUsage::kBwOverusing:
      if (rate_control_state_ != RateControlState::kRcDecrease) {
        rate_control_state_ = RateControlState::kRcDecrease;
      }
      break;
    case BandwidthUsage::kBwUnderusing:
      rate_control_state_ = RateControlState::kRcHold;
      break;
    default:
      RTC_NOTREACHED();
  }
}

}  // namespace webrtc

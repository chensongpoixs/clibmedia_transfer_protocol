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
				   date:  2025-09-31

					基于掉包带宽估计

 ******************************************************************************/


#ifndef _C__LOSS_BASED_BANDWIDTH_ESTIMATION_H_
#define _C__LOSS_BASED_BANDWIDTH_ESTIMATION_H_

#include <vector>

#include "libice/network_types.h"
#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace libmedia_transfer_protocol {

struct LossBasedControlConfig {
  explicit LossBasedControlConfig(/*const WebRtcKeyValueConfig* key_value_config*/);
  LossBasedControlConfig(const LossBasedControlConfig&);
  LossBasedControlConfig& operator=(const LossBasedControlConfig&) = default;
  ~LossBasedControlConfig();
  bool enabled;
  webrtc::FieldTrialParameter<double> min_increase_factor;
  webrtc::FieldTrialParameter<double> max_increase_factor;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> increase_low_rtt;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> increase_high_rtt;
  webrtc::FieldTrialParameter<double> decrease_factor;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> loss_window;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> loss_max_window;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> acknowledged_rate_max_window;
  webrtc::FieldTrialParameter<webrtc::DataRate> increase_offset;
  webrtc::FieldTrialParameter<webrtc::DataRate> loss_bandwidth_balance_increase;
  webrtc::FieldTrialParameter<webrtc::DataRate> loss_bandwidth_balance_decrease;
  webrtc::FieldTrialParameter<webrtc::DataRate> loss_bandwidth_balance_reset;
  webrtc::FieldTrialParameter<double> loss_bandwidth_balance_exponent;
  webrtc::FieldTrialParameter<bool> allow_resets;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> decrease_interval;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> loss_report_timeout;
};

// Estimates an upper BWE limit based on loss.
// It requires knowledge about lost packets and acknowledged bitrate.
// Ie, this class require transport feedback.
class LossBasedBandwidthEstimation {
 public:
  explicit LossBasedBandwidthEstimation(
      /*const WebRtcKeyValueConfig* key_value_config*/);
  // Returns the new estimate.
  webrtc::DataRate Update(webrtc::Timestamp at_time,
	  webrtc::DataRate min_bitrate,
                 webrtc::DataRate wanted_bitrate,
                 webrtc::TimeDelta last_round_trip_time);
  void UpdateAcknowledgedBitrate(webrtc::DataRate acknowledged_bitrate,
                                 webrtc::Timestamp at_time);
  void Initialize(webrtc::DataRate bitrate);
  bool Enabled() const { return config_.enabled; }
  // Returns true if LossBasedBandwidthEstimation is enabled and have
  // received loss statistics. Ie, this class require transport feedback.
  bool InUse() const {
    return Enabled() && last_loss_packet_report_.IsFinite();
  }
  void UpdateLossStatistics(const std::vector<libice::PacketResult>& packet_results,
	  webrtc::Timestamp at_time);
  webrtc::DataRate GetEstimate() const { return loss_based_bitrate_; }

 private:
  friend class GoogCcStatePrinter;
  void Reset(webrtc::DataRate bitrate);
  double loss_increase_threshold() const;
  double loss_decrease_threshold() const;
  double loss_reset_threshold() const;

  webrtc::DataRate decreased_bitrate() const;

  const LossBasedControlConfig config_;
  double average_loss_;
  double average_loss_max_;
  webrtc::DataRate loss_based_bitrate_;
  webrtc::DataRate acknowledged_bitrate_max_;
  webrtc::Timestamp acknowledged_bitrate_last_update_;
  webrtc::Timestamp time_last_decrease_;
  bool has_decreased_since_last_loss_report_;
  webrtc::Timestamp last_loss_packet_report_;
  double last_loss_ratio_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_LOSS_BASED_BANDWIDTH_ESTIMATION_H_

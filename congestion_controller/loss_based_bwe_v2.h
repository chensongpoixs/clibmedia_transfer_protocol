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
				   date:  2025-09-30

					基于掉包带宽估计

 ******************************************************************************/


#ifndef _C__LOSS_BASED_BWE_V2_H_
#define _C__LOSS_BASED_BWE_V2_H_

#include <cstddef>
#include <vector>

#include "absl/types/optional.h"
#include "api/array_view.h"
#include "libice/network_types.h"
#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"

namespace libmedia_transfer_protocol {

class LossBasedBweV2 {
 public:
  // Creates a disabled `LossBasedBweV2` if the
  // `key_value_config` is not valid.
  explicit LossBasedBweV2(/*const WebRtcKeyValueConfig* key_value_config*/);

  LossBasedBweV2(const LossBasedBweV2&) = delete;
  LossBasedBweV2& operator=(const LossBasedBweV2&) = delete;

  ~LossBasedBweV2() = default;

  bool IsEnabled() const;
  // Returns true iff a BWE can be calculated, i.e., the estimator has been
  // initialized with a BWE and then has received enough `PacketResult`s.
  bool IsReady() const;

  // Returns `DataRate::PlusInfinity` if no BWE can be calculated.
  webrtc::DataRate GetBandwidthEstimate() const;

  void SetAcknowledgedBitrate(webrtc::DataRate acknowledged_bitrate);
  void SetBandwidthEstimate(webrtc::DataRate bandwidth_estimate);

  void UpdateBandwidthEstimate(
      rtc::ArrayView<const libice::PacketResult> packet_results,
      webrtc::DataRate delay_based_estimate);

 private:
  struct ChannelParameters {
    double inherent_loss = 0.0;
   webrtc:: DataRate loss_limited_bandwidth = webrtc::DataRate::MinusInfinity();
  };

  struct Config {
    double bandwidth_rampup_upper_bound_factor = 0.0;
    double rampup_acceleration_max_factor = 0.0;
	webrtc::TimeDelta rampup_acceleration_maxout_time = webrtc::TimeDelta::Zero();
    std::vector<double> candidate_factors;
    double higher_bandwidth_bias_factor = 0.0;
    double higher_log_bandwidth_bias_factor = 0.0;
    double inherent_loss_lower_bound = 0.0;
	webrtc::DataRate inherent_loss_upper_bound_bandwidth_balance =
		webrtc::DataRate::MinusInfinity();
    double inherent_loss_upper_bound_offset = 0.0;
    double initial_inherent_loss_estimate = 0.0;
    int newton_iterations = 0;
    double newton_step_size = 0.0;
    bool append_acknowledged_rate_candidate = true;
    bool append_delay_based_estimate_candidate = false;
	webrtc::TimeDelta observation_duration_lower_bound = webrtc::TimeDelta::Zero();
    int observation_window_size = 0;
    double sending_rate_smoothing_factor = 0.0;
    double instant_upper_bound_temporal_weight_factor = 0.0;
	webrtc::DataRate instant_upper_bound_bandwidth_balance = webrtc::DataRate::MinusInfinity();
    double instant_upper_bound_loss_offset = 0.0;
    double temporal_weight_factor = 0.0;
  };

  struct Derivatives {
    double first = 0.0;
    double second = 0.0;
  };

  struct Observation {
    bool IsInitialized() const { return id != -1; }

    int num_packets = 0;
    int num_lost_packets = 0;
    int num_received_packets = 0;
	webrtc::DataRate sending_rate = webrtc::DataRate::MinusInfinity();
    int id = -1;
  };

  struct PartialObservation {
    int num_packets = 0;
    int num_lost_packets = 0;
	webrtc::DataSize size = webrtc::DataSize::Zero();
  };

  static absl::optional<Config> CreateConfig(/*
      const WebRtcKeyValueConfig* key_value_config*/);
  bool IsConfigValid() const;

  // Returns `0.0` if not enough loss statistics have been received.
  double GetAverageReportedLossRatio() const;
  std::vector<ChannelParameters> GetCandidates(
	  webrtc::DataRate delay_based_estimate) const;
  webrtc::DataRate GetCandidateBandwidthUpperBound() const;
  Derivatives GetDerivatives(const ChannelParameters& channel_parameters) const;
  double GetFeasibleInherentLoss(
      const ChannelParameters& channel_parameters) const;
  double GetInherentLossUpperBound(webrtc::DataRate bandwidth) const;
  double GetHighBandwidthBias(webrtc::DataRate bandwidth) const;
  double GetObjective(const ChannelParameters& channel_parameters) const;
  webrtc::DataRate GetSendingRate(webrtc::DataRate instantaneous_sending_rate) const;
  webrtc::DataRate GetInstantUpperBound() const;
  void CalculateInstantUpperBound();

  void CalculateTemporalWeights();
  void NewtonsMethodUpdate(ChannelParameters& channel_parameters) const;

  // Returns false if no observation was created.
  bool PushBackObservation(rtc::ArrayView<const libice::PacketResult> packet_results);

  absl::optional<webrtc::DataRate> acknowledged_bitrate_;
  absl::optional<Config> config_;
  ChannelParameters current_estimate_;
  int num_observations_ = 0;
  std::vector<Observation> observations_;
  PartialObservation partial_observation_;
  webrtc::Timestamp last_send_time_most_recent_observation_ = webrtc::Timestamp::PlusInfinity();
  webrtc::Timestamp last_time_estimate_reduced_ = webrtc::Timestamp::MinusInfinity();
  absl::optional<webrtc::DataRate> cached_instant_upper_bound_;
  std::vector<double> instant_upper_bound_temporal_weights_;
  std::vector<double> temporal_weights_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_LOSS_BASED_BWE_V2_H_

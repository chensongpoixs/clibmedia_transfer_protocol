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

				   Â³°ôÍÌÍÂÁ¿¹ÀËãÆ÷

 ******************************************************************************/


#ifndef _C__ROBUST_THROUGHPUT_ESTIMATOR_H_
#define _C__ROBUST_THROUGHPUT_ESTIMATOR_H_

#include <deque>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "libice/network_types.h"
//#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "libmedia_transfer_protocol/congestion_controller/acknowledged_bitrate_estimator_interface.h"

namespace libmedia_transfer_protocol {

class RobustThroughputEstimator : public AcknowledgedBitrateEstimatorInterface {
 public:
  explicit RobustThroughputEstimator(
      const RobustThroughputEstimatorSettings& settings);
  ~RobustThroughputEstimator() override;

  void IncomingPacketFeedbackVector(
      const std::vector<libice::PacketResult>& packet_feedback_vector) override;

  absl::optional<webrtc::DataRate> bitrate() const override;

  absl::optional<webrtc::DataRate> PeekRate() const override { return bitrate(); }
  void SetAlr(bool /*in_alr*/) override {}
  void SetAlrEndedTime(webrtc::Timestamp /*alr_ended_time*/) override {}

 private:
  const RobustThroughputEstimatorSettings settings_;
  std::deque<libice::PacketResult> window_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_ROBUST_THROUGHPUT_ESTIMATOR_H_

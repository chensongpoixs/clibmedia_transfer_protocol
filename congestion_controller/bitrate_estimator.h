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

				   贝叶斯估计出网络 吞吐量



 ******************************************************************************/


#ifndef _C_BITRATE_ESTIMATOR_H_
#define _C_BITRATE_ESTIMATOR_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "api/units/timestamp.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace libmedia_transfer_protocol {

// Computes a bayesian estimate of the throughput given acks containing
// the arrival time and payload size. Samples which are far from the current
// estimate or are based on few packets are given a smaller weight, as they
// are considered to be more likely to have been caused by, e.g., delay spikes
// unrelated to congestion.
class BitrateEstimator {
 public:
   explicit BitrateEstimator(/*const WebRtcKeyValueConfig* key_value_config*/)  ;
  virtual ~BitrateEstimator();
  virtual void Update(webrtc::Timestamp at_time, webrtc::DataSize amount, bool in_alr);

  virtual absl::optional<webrtc::DataRate> bitrate() const;
  absl::optional<webrtc::DataRate> PeekRate() const;

  virtual void ExpectFastRateChange();

 private:
  float UpdateWindow(int64_t now_ms,
                     int bytes,
                     int rate_window_ms,
                     bool* is_small_sample);
  int sum_;
  webrtc::FieldTrialConstrained<int> initial_window_ms_;
  webrtc::FieldTrialConstrained<int> noninitial_window_ms_;
  //不确定的大小 误差
  webrtc::FieldTrialParameter<double> uncertainty_scale_;
  webrtc::FieldTrialParameter<double> uncertainty_scale_in_alr_;
  webrtc::FieldTrialParameter<double> small_sample_uncertainty_scale_;
  webrtc::FieldTrialParameter<webrtc::DataSize> small_sample_threshold_;
  webrtc::FieldTrialParameter<webrtc::DataRate> uncertainty_symmetry_cap_;
  webrtc::FieldTrialParameter<webrtc::DataRate> estimate_floor_;
  // 累计的时间
  int64_t current_window_ms_;
  // 上一次更新时间
  int64_t prev_time_ms_;
  // 统计的数据的大小
  float bitrate_estimate_kbps_;
  //先验方差
  float bitrate_estimate_var_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_BITRATE_ESTIMATOR_H_

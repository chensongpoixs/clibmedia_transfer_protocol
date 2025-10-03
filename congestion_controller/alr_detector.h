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
				   date:  2025-10-01

				                                1、优化吞吐量估计 

				  ALR 应用限制区域检测器  ===>     2、优化基于延迟的带宽估计

				                                3、优化Probe
											  

 ******************************************************************************/



#ifndef _C__ALR_DETECTOR_H_
#define _C__ALR_DETECTOR_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>

#include "absl/types/optional.h"
#include "api/transport/webrtc_key_value_config.h"
#include "libmedia_transfer_protocol/pacing/interval_budget.h"
#include "rtc_base/experiments/alr_experiment.h"
#include "rtc_base/experiments/struct_parameters_parser.h"

namespace libmedia_transfer_protocol {

//class RtcEventLog;

struct AlrDetectorConfig {
  // Sent traffic ratio as a function of network capacity used to determine
  // application-limited region. ALR region start when bandwidth usage drops
  // below kAlrStartUsageRatio and ends when it raises above
  // kAlrEndUsageRatio. NOTE: This is intentionally conservative at the moment
  // until BW adjustments of application limited region is fine tuned.
  double bandwidth_usage_ratio = 0.65;   //当前网络带宽使用率
  double start_budget_level_ratio = 0.80;  // 当前厨余了的多少 进入ALR状态
  double stop_budget_level_ratio = 0.50; // 解除ALR模式
  std::unique_ptr<webrtc::StructParametersParser> Parser();
};
// Application limited region detector is a class that utilizes signals of
// elapsed time and bytes sent to estimate whether network traffic is
// currently limited by the application's ability to generate traffic.
//
// AlrDetector provides a signal that can be utilized to adjust
// estimate bandwidth.
// Note: This class is not thread-safe.
class AlrDetector {
 public:
  AlrDetector(AlrDetectorConfig config/*, RtcEventLog* event_log*/);
  explicit AlrDetector(/*const WebRtcKeyValueConfig* key_value_config*/);
  //AlrDetector(const WebRtcKeyValueConfig* key_value_config,
  //            RtcEventLog* event_log);
  ~AlrDetector();

  void OnBytesSent(size_t bytes_sent, int64_t send_time_ms);

  // Set current estimated bandwidth.
  void SetEstimatedBitrate(int bitrate_bps);

  // Returns time in milliseconds when the current application-limited region
  // started or empty result if the sender is currently not application-limited.
  absl::optional<int64_t> GetApplicationLimitedRegionStartTime() const;

 private:
  friend class GoogCcStatePrinter;
  const AlrDetectorConfig conf_;

  absl::optional<int64_t> last_send_time_ms_;

  IntervalBudget alr_budget_;
  absl::optional<int64_t> alr_started_time_ms_;

  //RtcEventLog* event_log_;
};
}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_ALR_DETECTOR_H_

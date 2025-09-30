
 /*****************************************************************************
				   Author: chensong
				   date:  2025-09-29


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
				   减低码率：  减法倍速 

 ******************************************************************************/

 
#ifndef _C_AIMD_RATE_CONTROL_H_
#define _C_AIMD_RATE_CONTROL_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "libice/network_types.h"
//#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "api/units/timestamp.h"
#include "libmedia_transfer_protocol/congestion_controller/link_capacity_estimator.h"
#include "libmedia_transfer_protocol/remote_bitrate_estimator/bwe_defines.h"
//#include "rtc_base/experiments/field_trial_parser.h"

namespace libmedia_transfer_protocol {
// A rate control implementation based on additive increases of
// bitrate when no over-use is detected and multiplicative decreases when
// over-uses are detected. When we think the available bandwidth has changes or
// is unknown, we will switch to a "slow-start mode" where we increase
// multiplicatively.
class AimdRateControl {
 public:
  explicit AimdRateControl(/*const WebRtcKeyValueConfig* key_value_config*/);
  AimdRateControl(/*const WebRtcKeyValueConfig* key_value_config,*/ bool send_side);
  ~AimdRateControl();

  // Returns true if the target bitrate has been initialized. This happens
  // either if it has been explicitly set via SetStartBitrate/SetEstimate, or if
  // we have measured a throughput.
  // 是否设置起始码流
  bool ValidEstimate() const;
  // 设置起始 码流
  void SetStartBitrate(webrtc::DataRate start_bitrate);
  void SetMinBitrate(webrtc::DataRate min_bitrate);
  webrtc::TimeDelta GetFeedbackInterval() const;

  // Returns true if the bitrate estimate hasn't been changed for more than
  // an RTT, or if the estimated_throughput is less than half of the current
  // estimate. Should be used to decide if we should reduce the rate further
  // when over-using.
   // 为防止码流降低过于频繁， 需要码流降低的频率
  // 两次码流降低的间隔， 要大于1RTT
  bool TimeToReduceFurther(webrtc::Timestamp at_time,
	  webrtc::DataRate estimated_throughput) const;
  // As above. To be used if overusing before we have measured a throughput.
   // 进一步码率是否需要降低
  bool InitialTimeToReduceFurther(webrtc::Timestamp at_time) const;

  // 最新码流
  webrtc::DataRate LatestEstimate() const;
  // 设置rtt的方法
  void SetRtt(webrtc::TimeDelta rtt);
  // 返回最新码流
  webrtc::DataRate Update(const libmedia_transfer_protocol ::RateControlInput* input, webrtc::Timestamp at_time);
  void SetInApplicationLimitedRegion(bool in_alr);
  // 设置码率
  void SetEstimate(webrtc::DataRate bitrate, webrtc::Timestamp at_time);
  void SetNetworkStateEstimate(
      const absl::optional<libice::NetworkStateEstimate>& estimate);

  // Returns the increase rate when used bandwidth is near the link capacity.
  // 返回每一秒钟 返回增加码流的大小
  double GetNearMaxIncreaseRateBpsPerSecond() const;
  // Returns the expected time between overuse signals (assuming steady state).
  webrtc::TimeDelta GetExpectedBandwidthPeriod() const;

 private:
  enum class RateControlState { 
	  kRcHold /*保存码流不变*/, 
	  kRcIncrease/*增加码流*/, 
	  kRcDecrease/*下降码流*/ 
  };

  friend class GoogCcStatePrinter;
  // Update the target bitrate based on, among other things, the current rate
  // control state, the current target bitrate and the estimated throughput.
  // When in the "increase" state the bitrate will be increased either
  // additively or multiplicatively depending on the rate control region. When
  // in the "decrease" state the bitrate will be decreased to slightly below the
  // current throughput. When in the "hold" state the bitrate will be kept
  // constant to allow built up queues to drain.
  // 调整码流的大小
  void ChangeBitrate(const libmedia_transfer_protocol::RateControlInput& input, webrtc::Timestamp at_time);

  // 当前码率限制 最小码流、最大码流  和链路容量大小限制
  webrtc::DataRate ClampBitrate(webrtc::DataRate new_bitrate) const;
  // 成性增加网络码流  1000bps
  webrtc::DataRate MultiplicativeRateIncrease(webrtc::Timestamp at_time,
	  webrtc::Timestamp last_ms,
	  webrtc::DataRate current_bitrate) const;
  webrtc::DataRate AdditiveRateIncrease(webrtc::Timestamp at_time, webrtc::Timestamp last_time) const;
  void UpdateChangePeriod(webrtc::Timestamp at_time);
  void ChangeState(const libmedia_transfer_protocol::RateControlInput& input, webrtc::Timestamp at_time);

  // 最小码流
  webrtc::DataRate min_configured_bitrate_;
   // 最大码流
  webrtc::DataRate max_configured_bitrate_;
  // 起始码率
  webrtc::DataRate current_bitrate_;
  // 记录上一次吞吐量码流
  webrtc::DataRate latest_estimated_throughput_;
  LinkCapacityEstimator link_capacity_;
  absl::optional<libice::NetworkStateEstimate> network_estimate_;
  // 码流控制 的状态机
  RateControlState rate_control_state_;
  // 上一次码流调整时间的记录
  webrtc::Timestamp time_last_bitrate_change_;
  // 最新码流降低时间记录
  webrtc::Timestamp time_last_bitrate_decrease_;
  // 第一次收到更新码流时间
  webrtc::Timestamp time_first_throughput_estimate_;
   // 起始 码流是否设置
  bool bitrate_is_initialized_;
  // 码流下降的倍数 0.85 
  double beta_;
  bool in_alr_;
  // 默认rtt 200ms
  webrtc::TimeDelta rtt_;
  const bool send_side_;
  const bool in_experiment_;
  // Allow the delay based estimate to only increase as long as application
  // limited region (alr) is not detected.
  const bool no_bitrate_increase_in_alr_;
  // Use estimated link capacity lower bound if it is higher than the
  // acknowledged rate when backing off due to overuse.
  const bool estimate_bounded_backoff_;
  // Use estimated link capacity upper bound as upper limit for increasing
  // bitrate over the acknowledged rate.
  const bool estimate_bounded_increase_;
  absl::optional<webrtc::DataRate> last_decrease_;
//  FieldTrialOptional<webrtc::TimeDelta> initial_backoff_interval_;
 // FieldTrialFlag link_capacity_fix_;
};
}  // namespace webrtc

#endif  // MODULES_REMOTE_BITRATE_ESTIMATOR_AIMD_RATE_CONTROL_H_


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


				   AIMD ��������   

				   �������ʣ�  �ӷ�
				   �������ʣ�  �������� 

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
  // �Ƿ�������ʼ����
  bool ValidEstimate() const;
  // ������ʼ ����
  void SetStartBitrate(webrtc::DataRate start_bitrate);
  void SetMinBitrate(webrtc::DataRate min_bitrate);
  webrtc::TimeDelta GetFeedbackInterval() const;

  // Returns true if the bitrate estimate hasn't been changed for more than
  // an RTT, or if the estimated_throughput is less than half of the current
  // estimate. Should be used to decide if we should reduce the rate further
  // when over-using.
   // Ϊ��ֹ�������͹���Ƶ���� ��Ҫ�������͵�Ƶ��
  // �����������͵ļ���� Ҫ����1RTT
  bool TimeToReduceFurther(webrtc::Timestamp at_time,
	  webrtc::DataRate estimated_throughput) const;
  // As above. To be used if overusing before we have measured a throughput.
   // ��һ�������Ƿ���Ҫ����
  bool InitialTimeToReduceFurther(webrtc::Timestamp at_time) const;

  // ��������
  webrtc::DataRate LatestEstimate() const;
  // ����rtt�ķ���
  void SetRtt(webrtc::TimeDelta rtt);
  // ������������
  webrtc::DataRate Update(const libmedia_transfer_protocol ::RateControlInput* input, webrtc::Timestamp at_time);
  void SetInApplicationLimitedRegion(bool in_alr);
  // ��������
  void SetEstimate(webrtc::DataRate bitrate, webrtc::Timestamp at_time);
  void SetNetworkStateEstimate(
      const absl::optional<libice::NetworkStateEstimate>& estimate);

  // Returns the increase rate when used bandwidth is near the link capacity.
  // ����ÿһ���� �������������Ĵ�С
  double GetNearMaxIncreaseRateBpsPerSecond() const;
  // Returns the expected time between overuse signals (assuming steady state).
  webrtc::TimeDelta GetExpectedBandwidthPeriod() const;

 private:
  enum class RateControlState { 
	  kRcHold /*������������*/, 
	  kRcIncrease/*��������*/, 
	  kRcDecrease/*�½�����*/ 
  };

  friend class GoogCcStatePrinter;
  // Update the target bitrate based on, among other things, the current rate
  // control state, the current target bitrate and the estimated throughput.
  // When in the "increase" state the bitrate will be increased either
  // additively or multiplicatively depending on the rate control region. When
  // in the "decrease" state the bitrate will be decreased to slightly below the
  // current throughput. When in the "hold" state the bitrate will be kept
  // constant to allow built up queues to drain.
  // ���������Ĵ�С
  void ChangeBitrate(const libmedia_transfer_protocol::RateControlInput& input, webrtc::Timestamp at_time);

  // ��ǰ�������� ��С�������������  ����·������С����
  webrtc::DataRate ClampBitrate(webrtc::DataRate new_bitrate) const;
  // ����������������  1000bps
  webrtc::DataRate MultiplicativeRateIncrease(webrtc::Timestamp at_time,
	  webrtc::Timestamp last_ms,
	  webrtc::DataRate current_bitrate) const;
  webrtc::DataRate AdditiveRateIncrease(webrtc::Timestamp at_time, webrtc::Timestamp last_time) const;
  void UpdateChangePeriod(webrtc::Timestamp at_time);
  void ChangeState(const libmedia_transfer_protocol::RateControlInput& input, webrtc::Timestamp at_time);

  // ��С����
  webrtc::DataRate min_configured_bitrate_;
   // �������
  webrtc::DataRate max_configured_bitrate_;
  // ��ʼ����
  webrtc::DataRate current_bitrate_;
  // ��¼��һ������������
  webrtc::DataRate latest_estimated_throughput_;
  LinkCapacityEstimator link_capacity_;
  absl::optional<libice::NetworkStateEstimate> network_estimate_;
  // �������� ��״̬��
  RateControlState rate_control_state_;
  // ��һ����������ʱ��ļ�¼
  webrtc::Timestamp time_last_bitrate_change_;
  // ������������ʱ���¼
  webrtc::Timestamp time_last_bitrate_decrease_;
  // ��һ���յ���������ʱ��
  webrtc::Timestamp time_first_throughput_estimate_;
   // ��ʼ �����Ƿ�����
  bool bitrate_is_initialized_;
  // �����½��ı��� 0.85 
  double beta_;
  bool in_alr_;
  // Ĭ��rtt 200ms
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

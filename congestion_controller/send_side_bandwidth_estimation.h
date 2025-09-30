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

				    ���ڵ���������� ===> 
					
					1. ǰ2s��ʹ�� �ӳٵĹ�������delay_based_bweģ���еĹ�������
					2. �����ӳٵĴ�����Ƶ�������Ϊ���ڵ����Ĵ�����Ƶ�����������
					3. ����������С������������ΪԼ������


 ******************************************************************************/


#ifndef _C__SEND_SIDE_BANDWIDTH_ESTIMATION_H_
#define _C__SEND_SIDE_BANDWIDTH_ESTIMATION_H_

#include <stdint.h>

#include <deque>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "libice/network_types.h"
#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_rate.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "libmedia_transfer_protocol/congestion_controller/loss_based_bandwidth_estimation.h"
#include "libmedia_transfer_protocol/congestion_controller/loss_based_bwe_v2.h"
#include "rtc_base/experiments/field_trial_parser.h"

namespace libmedia_transfer_protocol {

//class RtcEventLog;

class LinkCapacityTracker {
 public:
  LinkCapacityTracker();
  ~LinkCapacityTracker();
  // Call when a new delay-based estimate is available.
  void UpdateDelayBasedEstimate(webrtc::Timestamp at_time,
	  webrtc::DataRate delay_based_bitrate);
  void OnStartingRate(webrtc::DataRate start_rate);
  void OnRateUpdate(absl::optional<webrtc::DataRate> acknowledged,
	  webrtc::DataRate target,
	  webrtc::Timestamp at_time);
  void OnRttBackoff(webrtc::DataRate backoff_rate, webrtc::Timestamp at_time);
  webrtc::DataRate estimate() const;

 private:
	 webrtc::FieldTrialParameter<webrtc::TimeDelta> tracking_rate;
  double capacity_estimate_bps_ = 0;
  webrtc::Timestamp last_link_capacity_update_ = webrtc::Timestamp::MinusInfinity();
  webrtc::DataRate last_delay_based_estimate_ = webrtc::DataRate::PlusInfinity();
};

class RttBasedBackoff {
 public:
  explicit RttBasedBackoff(/*const WebRtcKeyValueConfig* key_value_config*/);
  ~RttBasedBackoff();
  void UpdatePropagationRtt(webrtc::Timestamp at_time, webrtc::TimeDelta propagation_rtt);
  webrtc::TimeDelta CorrectedRtt(webrtc::Timestamp at_time) const;

  webrtc::FieldTrialFlag disabled_;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> configured_limit_;
  webrtc::FieldTrialParameter<double> drop_fraction_;
  webrtc::FieldTrialParameter<webrtc::TimeDelta> drop_interval_;
  webrtc::FieldTrialParameter<webrtc::DataRate> bandwidth_floor_;

 public:
  webrtc::TimeDelta rtt_limit_;
  webrtc::Timestamp last_propagation_rtt_update_;
  webrtc::TimeDelta last_propagation_rtt_;
  webrtc::Timestamp last_packet_sent_;
};

class SendSideBandwidthEstimation {
 public:
 // SendSideBandwidthEstimation() = delete;
  SendSideBandwidthEstimation(/*const WebRtcKeyValueConfig* key_value_config,
                              RtcEventLog* event_log*/);
  ~SendSideBandwidthEstimation();

  void OnRouteChange();

  webrtc::DataRate target_rate() const;
  uint8_t fraction_loss() const { return last_fraction_loss_; }
  webrtc::TimeDelta round_trip_time() const { return last_round_trip_time_; }

  webrtc::DataRate GetEstimatedLinkCapacity() const;
  // Call periodically to update estimate.
  void UpdateEstimate(webrtc::Timestamp at_time);
  void OnSentPacket(const libice::SentPacket& sent_packet);
  void UpdatePropagationRtt(webrtc::Timestamp at_time, webrtc::TimeDelta propagation_rtt);

  // Call when we receive a RTCP message with TMMBR or REMB.
  void UpdateReceiverEstimate(webrtc::Timestamp at_time, webrtc::DataRate bandwidth);

  // Call when a new delay-based estimate is available.
  void UpdateDelayBasedEstimate(webrtc::Timestamp at_time, webrtc::DataRate bitrate);

  // Call when we receive a RTCP message with a ReceiveBlock.
  //���µ��� 
  void UpdatePacketsLost(int64_t packets_lost,
                         int64_t number_of_packets,
	  webrtc::Timestamp at_time);

  // Call when we receive a RTCP message with a ReceiveBlock.
  void UpdateRtt(webrtc::TimeDelta rtt, webrtc::Timestamp at_time);

  void SetBitrates(absl::optional<webrtc::DataRate> send_bitrate,
                   webrtc::DataRate min_bitrate,
                   webrtc::DataRate max_bitrate,
                   webrtc::Timestamp at_time);
  void SetSendBitrate(webrtc::DataRate bitrate, webrtc::Timestamp at_time);
  void SetMinMaxBitrate(webrtc::DataRate min_bitrate, webrtc::DataRate max_bitrate);
  int GetMinBitrate() const;
  void SetAcknowledgedRate(absl::optional<webrtc::DataRate> acknowledged_rate,
	  webrtc::Timestamp at_time);
  void IncomingPacketFeedbackVector(const libice::TransportPacketsFeedback& report);

 private:
  friend class GoogCcStatePrinter;

  enum UmaState { kNoUpdate, kFirstDone, kDone };

  bool IsInStartPhase(webrtc::Timestamp at_time) const;

  void UpdateUmaStatsPacketsLost(webrtc::Timestamp at_time, int packets_lost);

  // Updates history of min bitrates.
  // After this method returns min_bitrate_history_.front().second contains the
  // min bitrate used during last kBweIncreaseIntervalMs.
  void UpdateMinHistory(webrtc::Timestamp at_time);

  // Gets the upper limit for the target bitrate. This is the minimum of the
  // delay based limit, the receiver limit and the loss based controller limit.
  webrtc::DataRate GetUpperLimit() const;
  // Prints a warning if `bitrate` if sufficiently long time has past since last
  // warning.
  void MaybeLogLowBitrateWarning(webrtc::DataRate bitrate, webrtc::Timestamp at_time);
  // Stores an update to the event log if the loss rate has changed, the target
  // has changed, or sufficient time has passed since last stored event.
  void MaybeLogLossBasedEvent(webrtc::Timestamp at_time);

  // Cap `bitrate` to [min_bitrate_configured_, max_bitrate_configured_] and
  // set `current_bitrate_` to the capped value and updates the event log.
  //����Ŀǰ����  �����ӳٹ��Ƶ�  [min_bitrate, max_bitrate]
  void UpdateTargetBitrate(webrtc::DataRate bitrate, webrtc::Timestamp at_time);
  // Applies lower and upper bounds to the current target rate.
  // TODO(srte): This seems to be called even when limits haven't changed, that
  // should be cleaned up.
  void ApplyTargetLimits(webrtc::Timestamp at_time);

  bool LossBasedBandwidthEstimatorV1Enabled() const;
 // bool LossBasedBandwidthEstimatorV2Enabled() const;

  bool LossBasedBandwidthEstimatorV1ReadyForUse() const;
 // bool LossBasedBandwidthEstimatorV2ReadyForUse() const;

  RttBasedBackoff rtt_backoff_;
  LinkCapacityTracker link_capacity_;
  // ��С�����Ķ�������
  std::deque<std::pair<webrtc::Timestamp, webrtc::DataRate> > min_bitrate_history_;

  // incoming filters
  int lost_packets_since_last_loss_update_;
  // �ϴθ��°��ĸ���
  int expected_packets_since_last_loss_update_;

  absl::optional<webrtc::DataRate> acknowledged_rate_;
  // Ŀ������
  webrtc::DataRate current_target_;
  webrtc::DataRate last_logged_target_;
  // ������С����
  webrtc::DataRate min_bitrate_configured_;
  webrtc::DataRate max_bitrate_configured_;
  webrtc::Timestamp last_low_bitrate_log_;

  bool has_decreased_since_last_fraction_loss_;
  // ���¸��°���ʱ��
  webrtc::Timestamp last_loss_feedback_;
  // ���һ�θ��µ����ʵ�ʱ��
  webrtc::Timestamp last_loss_packet_report_;
  //������ [0, 255]
  uint8_t last_fraction_loss_;
  uint8_t last_logged_fraction_loss_;
  // rtt��ʱ��
  webrtc::TimeDelta last_round_trip_time_;

  // The max bitrate as set by the receiver in the call. This is typically
  // signalled using the REMB RTCP message and is used when we don't have any
  // send side delay based estimate.
  webrtc::DataRate receiver_limit_; // remb ģ��ı��� �°汾gccʹ�ò����� 
  //�����ӳٹ��Ƶ�����
  webrtc::DataRate delay_based_limit_;

  // ��¼��һ�ν���������ʱ��
  webrtc::Timestamp time_last_decrease_;
  // ��һ�θ��µ���
  webrtc::Timestamp first_report_time_;
  int initially_lost_packets_;
  webrtc::DataRate bitrate_at_2_seconds_;
  UmaState uma_update_state_;
  UmaState uma_rtt_state_;
  std::vector<bool> rampup_uma_stats_updated_;
 // RtcEventLog* const event_log_;
  webrtc::Timestamp last_rtc_event_log_;

  //�͵�����
  float low_loss_threshold_;
  //�ߵ�����
  float high_loss_threshold_;
  webrtc::DataRate bitrate_threshold_;
  LossBasedBandwidthEstimation loss_based_bandwidth_estimator_v1_;
  //LossBasedBweV2 loss_based_bandwidth_estimator_v2_;
  webrtc::FieldTrialFlag disable_receiver_limit_caps_only_;
};
}  // namespace webrtc
#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_SEND_SIDE_BANDWIDTH_ESTIMATION_H_

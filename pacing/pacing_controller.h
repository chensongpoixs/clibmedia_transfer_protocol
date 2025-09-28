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


#ifndef _C_MODULES_PACING_PACING_CONTROLLER_H_
#define _C_MODULES_PACING_PACING_CONTROLLER_H_

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/function_view.h"
//#include "api/rtc_event_log/rtc_event_log.h"
//#include "api/transport/field_trial_based_config.h"
#include "libice/network_types.h"
//#include "api/transport/webrtc_key_value_config.h"
#include "libmedia_transfer_protocol/pacing/bitrate_prober.h"
#include "libmedia_transfer_protocol/pacing/interval_budget.h"
#include "libmedia_transfer_protocol/pacing/round_robin_packet_queue.h"
#include "libmedia_transfer_protocol/pacing/rtp_packet_pacer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_sender.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/thread_annotations.h"

namespace libmedia_transfer_protocol {

// This class implements a leaky-bucket packet pacing algorithm. It handles the
// logic of determining which packets to send when, but the actual timing of
// the processing is done externally (e.g. PacedSender). Furthermore, the
// forwarding of packets when they are ready to be sent is also handled
// externally, via the PacedSendingController::PacketSender interface.
//
class PacingController {
 public:
  // Periodic mode uses the IntervalBudget class for tracking bitrate
  // budgets, and expected ProcessPackets() to be called a fixed rate,
  // e.g. every 5ms as implemented by PacedSender.
  // Dynamic mode allows for arbitrary time delta between calls to
  // ProcessPackets.
  enum class ProcessMode { kPeriodic, kDynamic };

  class PacketSender {
   public:
    virtual ~PacketSender() = default;
    virtual void SendPacket(std::unique_ptr<RtpPacketToSend> packet,
                            const libice::PacedPacketInfo& cluster_info) = 0;
    // Should be called after each call to SendPacket().
    virtual std::vector<std::unique_ptr<RtpPacketToSend>> FetchFec() = 0;
    virtual std::vector<std::unique_ptr<RtpPacketToSend>> GeneratePadding(
        webrtc::DataSize size) = 0;
  };

  // Expected max pacer delay. If ExpectedQueueTime() is higher than
  // this value, the packet producers should wait (eg drop frames rather than
  // encoding them). Bitrate sent may temporarily exceed target set by
  // UpdateBitrate() so that this limit will be upheld.
  static const webrtc::TimeDelta kMaxExpectedQueueLength;
  // Pacing-rate relative to our target send rate.
  // Multiplicative factor that is applied to the target bitrate to calculate
  // the number of bytes that can be transmitted per interval.
  // Increasing this factor will result in lower delays in cases of bitrate
  // overshoots from the encoder.
  static const float kDefaultPaceMultiplier;
  // If no media or paused, wake up at least every `kPausedProcessIntervalMs` in
  // order to send a keep-alive packet so we don't get stuck in a bad state due
  // to lack of feedback.
  static const webrtc::TimeDelta kPausedProcessInterval;

  static const webrtc::TimeDelta kMinSleepTime;

  PacingController(webrtc::Clock* clock,
                   PacketSender* packet_sender,
                  // RtcEventLog* event_log,
                   //const WebRtcKeyValueConfig* field_trials,
                   ProcessMode mode);

  ~PacingController();

  // Adds the packet to the queue and calls PacketRouter::SendPacket() when
  // it's time to send.
  void EnqueuePacket(std::unique_ptr<RtpPacketToSend> packet);

  void CreateProbeCluster(webrtc::DataRate bitrate, int cluster_id);

  void Pause();   // Temporarily pause all sending.
  void Resume();  // Resume sending packets.
  bool IsPaused() const;

  void SetCongestionWindow(webrtc::DataSize congestion_window_size);
  void UpdateOutstandingData(webrtc::DataSize outstanding_data);

  // Sets the pacing rates. Must be called once before packets can be sent.
  void SetPacingRates(webrtc::DataRate pacing_rate, webrtc::DataRate padding_rate);

  // Currently audio traffic is not accounted by pacer and passed through.
  // With the introduction of audio BWE audio traffic will be accounted for
  // the pacer budget calculation. The audio traffic still will be injected
  // at high priority.
  void SetAccountForAudioPackets(bool account_for_audio);
  void SetIncludeOverhead();

  void SetTransportOverhead(webrtc::DataSize overhead_per_packet);

  // Returns the time since the oldest queued packet was enqueued.
  webrtc::TimeDelta OldestPacketWaitTime() const;

  // Number of packets in the pacer queue.
  size_t QueueSizePackets() const;
  // Totals size of packets in the pacer queue.
  webrtc::DataSize QueueSizeData() const;

  // Current buffer level, i.e. max of media and padding debt.
  webrtc::DataSize CurrentBufferLevel() const;

  // Returns the time when the first packet was sent;
  absl::optional<webrtc::Timestamp> FirstSentPacketTime() const;

  // Returns the number of milliseconds it will take to send the current
  // packets in the queue, given the current size and bitrate, ignoring prio.
  webrtc::TimeDelta ExpectedQueueTime() const;

  void SetQueueTimeLimit(webrtc::TimeDelta limit);

  // Enable bitrate probing. Enabled by default, mostly here to simplify
  // testing. Must be called before any packets are being sent to have an
  // effect.
  void SetProbingEnabled(bool enabled);

  // Returns the next time we expect ProcessPackets() to be called.
  webrtc::Timestamp NextSendTime() const;

  // Check queue of pending packets and send them or padding packets, if budget
  // is available.
  void ProcessPackets();

  bool Congested() const;

  bool IsProbing() const;

 private:
  void EnqueuePacketInternal(std::unique_ptr<RtpPacketToSend> packet,
                             int priority);
  webrtc::TimeDelta UpdateTimeAndGetElapsed(webrtc::Timestamp now);
  bool ShouldSendKeepalive(webrtc::Timestamp now) const;

  // Updates the number of bytes that can be sent for the next time interval.
  void UpdateBudgetWithElapsedTime(webrtc::TimeDelta delta);
  void UpdateBudgetWithSentData(webrtc::DataSize size);

  webrtc::DataSize PaddingToAdd(webrtc::DataSize recommended_probe_size,
	  webrtc::DataSize data_sent) const;

  std::unique_ptr<RtpPacketToSend> GetPendingPacket(
      const libice::PacedPacketInfo& pacing_info,
	  webrtc::Timestamp target_send_time,
	  webrtc::Timestamp now);
  void OnPacketSent(RtpPacketMediaType packet_type,
	  webrtc::DataSize packet_size,
	  webrtc::Timestamp send_time);
  void OnPaddingSent(webrtc::DataSize padding_sent);

  webrtc::Timestamp CurrentTime() const;

  const ProcessMode mode_;
  webrtc::Clock* const clock_;
  PacketSender* const packet_sender_;
 // const std::unique_ptr<FieldTrialBasedConfig> fallback_field_trials_;
 // const WebRtcKeyValueConfig* field_trials_;

  const bool drain_large_queues_;
  const bool send_padding_if_silent_;
  const bool pace_audio_;
  const bool ignore_transport_overhead_;
  // In dynamic mode, indicates the target size when requesting padding,
  // expressed as a duration in order to adjust for varying padding rate.
  const webrtc::TimeDelta padding_target_duration_;

  webrtc::TimeDelta min_packet_limit_;

  webrtc::DataSize transport_overhead_per_packet_;

  // TODO(webrtc:9716): Remove this when we are certain clocks are monotonic.
  // The last millisecond timestamp returned by `clock_`.
  mutable webrtc::Timestamp last_timestamp_;
  bool paused_;

  // If `use_interval_budget_` is true, `media_budget_` and `padding_budget_`
  // will be used to track when packets can be sent. Otherwise the media and
  // padding debt counters will be used together with the target rates.

  // This is the media budget, keeping track of how many bits of media
  // we can pace out during the current interval.
  IntervalBudget media_budget_;
  // This is the padding budget, keeping track of how many bits of padding we're
  // allowed to send out during the current interval. This budget will be
  // utilized when there's no media to send.
  IntervalBudget padding_budget_;

 webrtc:: DataSize media_debt_;
 webrtc:: DataSize padding_debt_;
 webrtc:: DataRate media_rate_;
 webrtc:: DataRate padding_rate_;

  BitrateProber prober_;
  bool probing_send_failure_;

  webrtc::DataRate pacing_bitrate_;

  webrtc::Timestamp last_process_time_;
  webrtc::Timestamp last_send_time_;
  absl::optional<webrtc::Timestamp> first_sent_packet_time_;

  RoundRobinPacketQueue packet_queue_;
  uint64_t packet_counter_;

  webrtc::DataSize congestion_window_size_;
  webrtc::DataSize outstanding_data_;

  webrtc::TimeDelta queue_time_limit;
  bool account_for_audio_;
  bool include_overhead_;
};
}  // namespace webrtc

#endif  // MODULES_PACING_PACING_CONTROLLER_H_

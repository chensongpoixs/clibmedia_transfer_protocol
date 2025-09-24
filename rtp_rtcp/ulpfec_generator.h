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
				   date:  2025-09-24



 ******************************************************************************/



#ifndef _C_MODULES_RTP_RTCP_SOURCE_ULPFEC_GENERATOR_H_
#define _C_MODULES_RTP_RTCP_SOURCE_ULPFEC_GENERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <memory>
#include <vector>

#include "modules/include/module_fec_types.h"
#include "libmedia_transfer_protocol/rtp_rtcp/forward_error_correction.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_fec_generator.h"
#include "rtc_base/race_checker.h"
#include "rtc_base/rate_statistics.h"
#include "rtc_base/synchronization/mutex.h"

namespace libmedia_transfer_protocol {

//class FlexfecSender;

class UlpfecGenerator : public VideoFecGenerator {
  friend class FlexfecSender;

 public:
  UlpfecGenerator(int red_payload_type, int ulpfec_payload_type, webrtc::Clock* clock);
  ~UlpfecGenerator();

  FecType GetFecType() const override {
    return VideoFecGenerator::FecType::kUlpFec;
  }
  absl::optional<uint32_t> FecSsrc() override { return absl::nullopt; }

  void SetProtectionParameters(const webrtc::FecProtectionParams& delta_params,
                               const webrtc::FecProtectionParams& key_params) override;

  // Adds a media packet to the internal buffer. When enough media packets
  // have been added, the FEC packets are generated and stored internally.
  // These FEC packets are then obtained by calling GetFecPacketsAsRed().
  void AddPacketAndGenerateFec(const RtpPacketToSend& packet) override;

  // Returns the overhead, per packet, for FEC (and possibly RED).
  size_t MaxPacketOverhead() const override;

  std::vector<std::unique_ptr<RtpPacketToSend>> GetFecPackets() override;

  // Current rate of FEC packets generated, including all RTP-level headers.
  webrtc::DataRate CurrentFecRate() const override;

  absl::optional<RtpState> GetRtpState() override { return absl::nullopt; }

  // Currently used protection params.
  const webrtc::FecProtectionParams& CurrentParams() const;

 private:
  struct Params {
    Params();
    Params(webrtc::FecProtectionParams delta_params,
		webrtc::FecProtectionParams keyframe_params);

	webrtc::FecProtectionParams delta_params;
	webrtc::FecProtectionParams keyframe_params;
  };

  UlpfecGenerator(std::unique_ptr<ForwardErrorCorrection> fec, webrtc::Clock* clock);

  // Overhead is defined as relative to the number of media packets, and not
  // relative to total number of packets. This definition is inherited from the
  // protection factor produced by video_coding module and how the FEC
  // generation is implemented.
  int Overhead() const;

  // Returns true if the excess overhead (actual - target) for the FEC is below
  // the amount `kMaxExcessOverhead`. This effects the lower protection level
  // cases and low number of media packets/frame. The target overhead is given
  // by `params_.fec_rate`, and is only achievable in the limit of large number
  // of media packets.
  bool ExcessOverheadBelowMax() const;

  // Returns true if the number of added media packets is at least
  // `min_num_media_packets_`. This condition tries to capture the effect
  // that, for the same amount of protection/overhead, longer codes
  // (e.g. (2k,2m) vs (k,m)) are generally more effective at recovering losses.
  bool MinimumMediaPacketsReached() const;

  void ResetState();

  const int red_payload_type_;
  const int ulpfec_payload_type_;
  webrtc::Clock* const clock_;

  rtc::RaceChecker race_checker_;
  const std::unique_ptr<ForwardErrorCorrection> fec_
      RTC_GUARDED_BY(race_checker_);
  ForwardErrorCorrection::PacketList media_packets_
      RTC_GUARDED_BY(race_checker_);
  absl::optional<RtpPacketToSend> last_media_packet_
      RTC_GUARDED_BY(race_checker_);
  std::list<ForwardErrorCorrection::Packet*> generated_fec_packets_
      RTC_GUARDED_BY(race_checker_);
  int num_protected_frames_ RTC_GUARDED_BY(race_checker_);
  int min_num_media_packets_ RTC_GUARDED_BY(race_checker_);
  Params current_params_ RTC_GUARDED_BY(race_checker_);
  bool media_contains_keyframe_ RTC_GUARDED_BY(race_checker_);

  mutable webrtc::Mutex mutex_;
  absl::optional<Params> pending_params_ RTC_GUARDED_BY(mutex_);
  webrtc::RateStatistics fec_bitrate_ RTC_GUARDED_BY(mutex_);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_ULPFEC_GENERATOR_H_

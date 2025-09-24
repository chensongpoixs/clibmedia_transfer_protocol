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

#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_REPORTS_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_REPORTS_H_

#include <vector>

#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/dlrr.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/rrtr.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/target_bitrate.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;

// From RFC 3611: RTP Control Protocol Extended Reports (RTCP XR).
class ExtendedReports : public RtcpPacket {
 public:
  static constexpr uint8_t kPacketType = 207;
  static constexpr size_t kMaxNumberOfDlrrItems = 50;

  ExtendedReports();
  ExtendedReports(const ExtendedReports& xr);
  ~ExtendedReports() override;

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& packet);

  void SetRrtr(const Rrtr& rrtr);
  bool AddDlrrItem(const ReceiveTimeInfo& time_info);
  void SetTargetBitrate(const TargetBitrate& target_bitrate);

  const absl::optional<Rrtr>& rrtr() const { return rrtr_block_; }
  const Dlrr& dlrr() const { return dlrr_block_; }
  const absl::optional<TargetBitrate>& target_bitrate() const {
    return target_bitrate_;
  }

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;

 private:
  static constexpr size_t kXrBaseLength = 4;

  size_t RrtrLength() const { return rrtr_block_ ? Rrtr::kLength : 0; }
  size_t DlrrLength() const { return dlrr_block_.BlockLength(); }
  size_t TargetBitrateLength() const;

  void ParseRrtrBlock(const uint8_t* block, uint16_t block_length);
  void ParseDlrrBlock(const uint8_t* block, uint16_t block_length);
  void ParseTargetBitrateBlock(const uint8_t* block, uint16_t block_length);

  absl::optional<Rrtr> rrtr_block_;
  Dlrr dlrr_block_;  // Dlrr without items treated same as no dlrr block.
  absl::optional<TargetBitrate> target_bitrate_;
};
}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_REPORTS_H_

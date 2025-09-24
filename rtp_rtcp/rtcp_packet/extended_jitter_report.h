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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_JITTER_REPORT_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_JITTER_REPORT_H_

#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;

class ExtendedJitterReport : public RtcpPacket {
 public:
  static constexpr uint8_t kPacketType = 195;
  static constexpr size_t kMaxNumberOfJitterValues = 0x1f;

  ExtendedJitterReport();
  ~ExtendedJitterReport() override;

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& packet);

  bool SetJitterValues(std::vector<uint32_t> jitter_values);

  const std::vector<uint32_t>& jitter_values() {
    return inter_arrival_jitters_;
  }

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;

 private:
  static constexpr size_t kJitterSizeBytes = 4;

  std::vector<uint32_t> inter_arrival_jitters_;
};

}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_EXTENDED_JITTER_REPORT_H_

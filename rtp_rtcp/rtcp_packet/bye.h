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
				   date:  2025-09-21



 ******************************************************************************/


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_BYE_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_BYE_H_

#include <string>
#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;

class Bye : public RtcpPacket {
 public:
  static constexpr uint8_t kPacketType = 203;

  Bye();
  ~Bye() override;

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& packet);

  bool SetCsrcs(std::vector<uint32_t> csrcs);
  void SetReason(std::string reason);

  const std::vector<uint32_t>& csrcs() const { return csrcs_; }
  const std::string& reason() const { return reason_; }

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;

 private:
  static const int kMaxNumberOfCsrcs = 0x1f - 1;  // First item is sender SSRC.

  std::vector<uint32_t> csrcs_;
  std::string reason_;
};

}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_BYE_H_

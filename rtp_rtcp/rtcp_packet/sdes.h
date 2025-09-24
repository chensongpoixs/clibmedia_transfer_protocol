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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_SDES_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_SDES_H_

#include <string>
#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;
// Source Description (SDES) (RFC 3550).
class Sdes : public RtcpPacket {
 public:
  struct Chunk {
    uint32_t ssrc;
    std::string cname;
  };
  static constexpr uint8_t kPacketType = 202;
  static constexpr size_t kMaxNumberOfChunks = 0x1f;

  Sdes();
  ~Sdes() override;

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& packet);

  bool AddCName(uint32_t ssrc, std::string cname);

  const std::vector<Chunk>& chunks() const { return chunks_; }

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;

 private:
  std::vector<Chunk> chunks_;
  size_t block_length_;
};
}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_SDES_H_

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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_COMPOUND_PACKET_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_COMPOUND_PACKET_H_

#include <memory>
#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "rtc_base/constructor_magic.h"

namespace libmedia_transfer_protocol {
namespace rtcp {

class CompoundPacket : public RtcpPacket {
 public:
  CompoundPacket();
  ~CompoundPacket() override;

  void Append(std::unique_ptr<RtcpPacket> packet);

  // Size of this packet in bytes (i.e. total size of nested packets).
  size_t BlockLength() const override;
  // Returns true if all calls to Create succeeded.
  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;

 protected:
  std::vector<std::unique_ptr<RtcpPacket>> appended_packets_;

 private:
  RTC_DISALLOW_COPY_AND_ASSIGN(CompoundPacket);
};

}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_COMPOUND_PACKET_H_

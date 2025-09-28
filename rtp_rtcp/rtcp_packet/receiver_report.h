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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RECEIVER_REPORT_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RECEIVER_REPORT_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/report_block.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;

class ReceiverReport : public RtcpPacket {
 public:
  static constexpr uint8_t kPacketType = 201;
  static constexpr size_t kMaxNumberOfReportBlocks = 0x1f;

  ReceiverReport();
  ReceiverReport(const ReceiverReport&);
  ~ReceiverReport() override;

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& packet);

  bool AddReportBlock(const ReportBlock& block);
  bool SetReportBlocks(std::vector<ReportBlock> blocks);

  const std::vector<ReportBlock>& report_blocks() const {
    return report_blocks_;
  }

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;
  std::string ToString() const;
 private:
  static const size_t kRrBaseLength = 4;

  std::vector<ReportBlock> report_blocks_;
};

}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RECEIVER_REPORT_H_

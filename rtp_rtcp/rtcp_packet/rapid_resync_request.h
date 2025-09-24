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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RAPID_RESYNC_REQUEST_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RAPID_RESYNC_REQUEST_H_

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/rtpfb.h"

namespace libmedia_transfer_protocol {
namespace rtcp {
class CommonHeader;

// draft-perkins-avt-rapid-rtp-sync-03
class RapidResyncRequest : public Rtpfb {
 public:
  static constexpr uint8_t kFeedbackMessageType = 5;

  RapidResyncRequest() {}
  ~RapidResyncRequest() override {}

  // Parse assumes header is already parsed and validated.
  bool Parse(const CommonHeader& header);

  size_t BlockLength() const override;

  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              PacketReadyCallback callback) const override;
};
}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RAPID_RESYNC_REQUEST_H_

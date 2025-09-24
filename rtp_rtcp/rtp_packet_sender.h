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


#ifndef _C_MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_
#define _C_MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_

#include <memory>
#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"

namespace libmedia_transfer_protocol {

class RtpPacketSender {
 public:
  virtual ~RtpPacketSender() = default;

  // Insert a set of packets into queue, for eventual transmission. Based on the
  // type of packets, they will be prioritized and scheduled relative to other
  // packets and the current target send rate.
  virtual void EnqueuePackets(
      std::vector<std::unique_ptr<RtpPacketToSend>> packets) = 0;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_

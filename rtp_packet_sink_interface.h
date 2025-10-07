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


#ifndef _C_CALL_RTP_PACKET_SINK_INTERFACE_H_
#define _C_CALL_RTP_PACKET_SINK_INTERFACE_H_
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"


namespace libmedia_transfer_protocol {

//class RtpPacketReceived;

// This class represents a receiver of already parsed RTP packets.
class RtpPacketSinkInterface {
 public:
  virtual ~RtpPacketSinkInterface() = default;
  virtual void OnRtpPacket(const RtpPacketReceived& packet) = 0;
};

}  // namespace webrtc

#endif  // CALL_RTP_PACKET_SINK_INTERFACE_H_

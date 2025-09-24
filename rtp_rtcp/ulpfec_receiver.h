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




#ifndef _C_MODULES_RTP_RTCP_INCLUDE_ULPFEC_RECEIVER_H_
#define _C_MODULES_RTP_RTCP_INCLUDE_ULPFEC_RECEIVER_H_

#include <memory>

#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_parameters.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"

namespace libmedia_transfer_protocol {

struct FecPacketCounter {
  FecPacketCounter() = default;
  size_t num_packets = 0;  // Number of received packets.
  size_t num_bytes = 0;
  size_t num_fec_packets = 0;  // Number of received FEC packets.
  size_t num_recovered_packets =
      0;  // Number of recovered media packets using FEC.
  int64_t first_packet_time_ms = -1;  // Time when first packet is received.
};

class UlpfecReceiver {
 public:
  static std::unique_ptr<UlpfecReceiver> Create(
      uint32_t ssrc,
      RecoveredPacketReceiver* callback,
      rtc::ArrayView<const RtpExtension> extensions);

  virtual ~UlpfecReceiver() {}

  // Takes a RED packet, strips the RED header, and adds the resulting
  // "virtual" RTP packet(s) into the internal buffer.
  //
  // TODO(brandtr): Set `ulpfec_payload_type` during constructor call,
  // rather than as a parameter here.
  virtual bool AddReceivedRedPacket(const RtpPacketReceived& rtp_packet,
                                    uint8_t ulpfec_payload_type) = 0;

  // Sends the received packets to the FEC and returns all packets
  // (both original media and recovered) through the callback.
  virtual int32_t ProcessReceivedFec() = 0;

  // Returns a counter describing the added and recovered packets.
  virtual FecPacketCounter GetPacketCounter() const = 0;

  virtual void SetRtpExtensions(
      rtc::ArrayView<const RtpExtension> extensions) = 0;
};
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_INCLUDE_ULPFEC_RECEIVER_H_

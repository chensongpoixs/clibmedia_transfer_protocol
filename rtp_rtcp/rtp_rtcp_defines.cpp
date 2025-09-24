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


#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"

#include <ctype.h>
#include <string.h>

#include <type_traits>

#include "absl/algorithm/container.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet.h"

namespace libmedia_transfer_protocol {

namespace {
constexpr size_t kMidRsidMaxSize = 16;

// Check if passed character is a "token-char" from RFC 4566.
bool IsTokenChar(char ch) {
  return ch == 0x21 || (ch >= 0x23 && ch <= 0x27) || ch == 0x2a || ch == 0x2b ||
         ch == 0x2d || ch == 0x2e || (ch >= 0x30 && ch <= 0x39) ||
         (ch >= 0x41 && ch <= 0x5a) || (ch >= 0x5e && ch <= 0x7e);
}
}  // namespace

bool IsLegalMidName(absl::string_view name) {
  return (name.size() <= kMidRsidMaxSize && !name.empty() &&
          absl::c_all_of(name, IsTokenChar));
}

bool IsLegalRsidName(absl::string_view name) {
  return (name.size() <= kMidRsidMaxSize && !name.empty() &&
          absl::c_all_of(name, isalnum));
}

StreamDataCounters::StreamDataCounters() : first_packet_time_ms(-1) {}

RtpPacketCounter::RtpPacketCounter(const RtpPacket& packet)
    : header_bytes(packet.headers_size()),
      payload_bytes(packet.payload_size()),
      padding_bytes(packet.padding_size()),
      packets(1) {}

void RtpPacketCounter::AddPacket(const RtpPacket& packet) {
  ++packets;
  header_bytes += packet.headers_size();
  padding_bytes += packet.padding_size();
  payload_bytes += packet.payload_size();
}

}  // namespace webrtc

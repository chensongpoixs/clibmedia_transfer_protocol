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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RRTR_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RRTR_H_

#include <stddef.h>
#include <stdint.h>

#include "system_wrappers/include/ntp_time.h"

namespace libmedia_transfer_protocol {
namespace rtcp {

class Rrtr {
 public:
  static const uint8_t kBlockType = 4;
  static const uint16_t kBlockLength = 2;
  static const size_t kLength = 4 * (kBlockLength + 1);  // 12

  Rrtr() {}
  Rrtr(const Rrtr&) = default;
  ~Rrtr() {}

  Rrtr& operator=(const Rrtr&) = default;

  void Parse(const uint8_t* buffer);

  // Fills buffer with the Rrtr.
  // Consumes Rrtr::kLength bytes.
  void Create(uint8_t* buffer) const;

  void SetNtp(webrtc::NtpTime ntp) { ntp_ = ntp; }

  webrtc::NtpTime ntp() const { return ntp_; }

 private:
  webrtc::NtpTime ntp_;
};

}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_RRTR_H_

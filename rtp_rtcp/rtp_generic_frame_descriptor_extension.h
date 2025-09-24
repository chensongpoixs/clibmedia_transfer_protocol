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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTP_GENERIC_FRAME_DESCRIPTOR_EXTENSION_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTP_GENERIC_FRAME_DESCRIPTOR_EXTENSION_H_

#include <stddef.h>
#include <stdint.h>

#include "absl/strings/string_view.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_parameters.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_generic_frame_descriptor.h"

namespace libmedia_transfer_protocol {

class RtpGenericFrameDescriptorExtension00 {
 public:
  using value_type = RtpGenericFrameDescriptor;
  static constexpr RTPExtensionType kId = kRtpExtensionGenericFrameDescriptor00;
  static constexpr absl::string_view Uri() {
    return RtpExtension::kGenericFrameDescriptorUri00;
  }
  static constexpr int kMaxSizeBytes = 16;

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    RtpGenericFrameDescriptor* descriptor);
  static size_t ValueSize(const RtpGenericFrameDescriptor& descriptor);
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const RtpGenericFrameDescriptor& descriptor);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTP_GENERIC_FRAME_DESCRIPTOR_EXTENSION_H_

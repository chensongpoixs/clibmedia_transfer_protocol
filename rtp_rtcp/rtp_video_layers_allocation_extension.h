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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_LAYERS_ALLOCATION_EXTENSION_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_LAYERS_ALLOCATION_EXTENSION_H_

#include "absl/strings/string_view.h"
#include "libmedia_transfer_protocol/rtp_parameters.h"
#include "libmedia_codec/video_layers_allocation.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"

namespace libmedia_transfer_protocol {

// TODO(bugs.webrtc.org/12000): Note that this extensions is being developed and
// the wire format will likely change.
class RtpVideoLayersAllocationExtension {
 public:
  using value_type = libmedia_codec::VideoLayersAllocation;
  static constexpr RTPExtensionType kId = kRtpExtensionVideoLayersAllocation;
  static constexpr absl::string_view Uri() {
    return RtpExtension::kVideoLayersAllocationUri;
  }

  static bool Parse(rtc::ArrayView<const uint8_t> data,
	  libmedia_codec::VideoLayersAllocation* allocation);
  static size_t ValueSize(const libmedia_codec::VideoLayersAllocation& allocation);
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const libmedia_codec::VideoLayersAllocation& allocation);
};

}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_LAYERS_ALLOCATION_EXTENSION_H_

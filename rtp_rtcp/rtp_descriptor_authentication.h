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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTP_DESCRIPTOR_AUTHENTICATION_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTP_DESCRIPTOR_AUTHENTICATION_H_

#include <cstdint>
#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtp_video_header.h"

namespace libmedia_transfer_protocol {

// Converts frame dependencies into array of bytes for authentication.
std::vector<uint8_t> RtpDescriptorAuthentication(
    const RTPVideoHeader& rtp_video_header);

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTP_DESCRIPTOR_AUTHENTICATION_H_

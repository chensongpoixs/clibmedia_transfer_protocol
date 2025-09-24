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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_CREATE_VIDEO_RTP_DEPACKETIZER_H_
#define _C_MODULES_RTP_RTCP_SOURCE_CREATE_VIDEO_RTP_DEPACKETIZER_H_

#include <memory>

#include "api/video/video_codec_type.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer.h"

namespace libmedia_transfer_protocol {

std::unique_ptr<VideoRtpDepacketizer> CreateVideoRtpDepacketizer(
    webrtc::VideoCodecType codec);

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_CREATE_VIDEO_RTP_DEPACKETIZER_H_

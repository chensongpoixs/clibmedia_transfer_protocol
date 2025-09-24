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

#include "libmedia_transfer_protocol/rtp_rtcp/create_video_rtp_depacketizer.h"

#include <memory>

#include "api/video/video_codec_type.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_av1.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_generic.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_h264.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_vp8.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_vp9.h"

namespace libmedia_transfer_protocol {

std::unique_ptr<VideoRtpDepacketizer> CreateVideoRtpDepacketizer(
    webrtc::VideoCodecType codec) {
  switch (codec) {
    case webrtc::kVideoCodecH264:
      return std::make_unique<VideoRtpDepacketizerH264>();
    case webrtc::kVideoCodecVP8:
      return std::make_unique<VideoRtpDepacketizerVp8>();
    case webrtc::kVideoCodecVP9:
      return std::make_unique<VideoRtpDepacketizerVp9>();
    case webrtc::kVideoCodecAV1:
      return std::make_unique<VideoRtpDepacketizerAv1>();
    case webrtc::kVideoCodecGeneric:
    case webrtc::kVideoCodecMultiplex:
      return std::make_unique<VideoRtpDepacketizerGeneric>();
  }
  RTC_CHECK_NOTREACHED();
}

}  // namespace webrtc

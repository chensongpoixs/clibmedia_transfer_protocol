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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_VIDEO_RTP_DEPACKETIZER_H_
#define _C_MODULES_RTP_RTCP_SOURCE_VIDEO_RTP_DEPACKETIZER_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "api/array_view.h"
#include "api/scoped_refptr.h"
#include "libmedia_codec/encoded_image.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_video_header.h"
#include "rtc_base/copy_on_write_buffer.h"

namespace libmedia_transfer_protocol {

class VideoRtpDepacketizer {
 public:
  struct ParsedRtpPayload {
    RTPVideoHeader video_header;
    rtc::CopyOnWriteBuffer video_payload;
  };

  virtual ~VideoRtpDepacketizer() = default;
  virtual absl::optional<ParsedRtpPayload> Parse(
      rtc::CopyOnWriteBuffer rtp_payload) = 0;
  virtual rtc::scoped_refptr<libmedia_codec::EncodedImageBuffer> AssembleFrame(
      rtc::ArrayView<const rtc::ArrayView<const uint8_t>> rtp_payloads);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_VIDEO_RTP_DEPACKETIZER_H_

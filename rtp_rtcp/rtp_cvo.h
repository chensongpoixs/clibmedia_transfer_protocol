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

#ifndef _C_MODULES_RTP_RTCP_INCLUDE_RTP_CVO_H_
#define _C_MODULES_RTP_RTCP_INCLUDE_RTP_CVO_H_

#include "api/video/video_rotation.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {

// Please refer to http://www.etsi.org/deliver/etsi_ts/126100_126199/126114/
// 12.07.00_60/ts_126114v120700p.pdf Section 7.4.5. The rotation of a frame is
// the clockwise angle the frames must be rotated in order to display the frames
// correctly if the display is rotated in its natural orientation.
inline uint8_t ConvertVideoRotationToCVOByte(libmedia_codec::VideoRotation rotation) {
  switch (rotation) {
    case libmedia_codec::kVideoRotation_0:
      return 0;
    case libmedia_codec::kVideoRotation_90:
      return 1;
    case libmedia_codec::kVideoRotation_180:
      return 2;
    case libmedia_codec::kVideoRotation_270:
      return 3;
  }
  RTC_NOTREACHED();
  return 0;
}

inline libmedia_codec::VideoRotation ConvertCVOByteToVideoRotation(uint8_t cvo_byte) {
  // CVO byte: |0 0 0 0 C F R R|.
  const uint8_t rotation_bits = cvo_byte & 0x3;
  switch (rotation_bits) {
    case 0:
      return libmedia_codec::kVideoRotation_0;
    case 1:
      return libmedia_codec::kVideoRotation_90;
    case 2:
      return libmedia_codec::kVideoRotation_180;
    case 3:
      return libmedia_codec::kVideoRotation_270;
    default:
      RTC_NOTREACHED();
      return libmedia_codec::kVideoRotation_0;
  }
}

}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_INCLUDE_RTP_CVO_H_

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
				   date:  2025-10-06



 ******************************************************************************/



#ifndef _C_MODULES_VIDEO_CODING_RTP_GENERIC_REF_FINDER_H_
#define _C_MODULES_VIDEO_CODING_RTP_GENERIC_REF_FINDER_H_

#include <memory>

#include "libmedia_codec/frame_object.h"
#include "libmedia_transfer_protocol/rtp_frame_reference_finder.h"

namespace libmedia_transfer_protocol {

class RtpGenericFrameRefFinder {
 public:
  RtpGenericFrameRefFinder() = default;

  RtpFrameReferenceFinder::ReturnVector ManageFrame(
      std::unique_ptr<libmedia_codec::RtpFrameObject> frame,
      const RTPVideoHeader::GenericDescriptorInfo& descriptor);
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_RTP_GENERIC_REF_FINDER_H_

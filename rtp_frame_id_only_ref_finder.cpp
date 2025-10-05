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
				   date:  2025-09-23



 ******************************************************************************/


#include "libmedia_transfer_protocol/rtp_frame_id_only_ref_finder.h"

#include <utility>

#include "rtc_base/logging.h"

namespace libmedia_transfer_protocol {

RtpFrameReferenceFinder::ReturnVector RtpFrameIdOnlyRefFinder::ManageFrame(
    std::unique_ptr<libmedia_codec::RtpFrameObject> frame,
    int frame_id) {
  frame->SetSpatialIndex(0);
  frame->SetId(unwrapper_.Unwrap(frame_id & (kFrameIdLength - 1)));
  frame->num_references =
      frame->frame_type() == libmedia_codec::VideoFrameType::kVideoFrameKey ? 0 : 1;
  frame->references[0] = frame->Id() - 1;

  RtpFrameReferenceFinder::ReturnVector res;
  res.push_back(std::move(frame));
  return res;
}

}  // namespace webrtc

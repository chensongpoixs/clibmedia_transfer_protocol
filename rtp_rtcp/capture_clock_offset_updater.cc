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


#include "libmedia_transfer_protocol/rtp_rtcp/capture_clock_offset_updater.h"

namespace libmedia_transfer_protocol {

absl::optional<int64_t>
CaptureClockOffsetUpdater::AdjustEstimatedCaptureClockOffset(
    absl::optional<int64_t> remote_capture_clock_offset) const {
  if (remote_capture_clock_offset == absl::nullopt ||
      remote_to_local_clock_offset_ == absl::nullopt) {
    return absl::nullopt;
  }

  // Do calculations as "unsigned" to make overflows deterministic.
  return static_cast<uint64_t>(*remote_capture_clock_offset) +
         static_cast<uint64_t>(*remote_to_local_clock_offset_);
}

void CaptureClockOffsetUpdater::SetRemoteToLocalClockOffset(
    absl::optional<int64_t> offset_q32x32) {
  remote_to_local_clock_offset_ = offset_q32x32;
}

}  // namespace webrtc

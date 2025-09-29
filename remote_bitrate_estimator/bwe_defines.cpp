
/*****************************************************************************
				  Author: chensong
				  date:  2025-09-29


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
				  date:  2025-09-29




******************************************************************************/




#include "libmedia_transfer_protocol/remote_bitrate_estimator/bwe_defines.h"

#include "system_wrappers/include/field_trial.h"

namespace libmedia_transfer_protocol {

const char kBweTypeHistogram[] = "WebRTC.BWE.Types";

namespace congestion_controller {
int GetMinBitrateBps() {
  constexpr int kMinBitrateBps = 5000;
  return kMinBitrateBps;
}

webrtc::DataRate GetMinBitrate() {
  return webrtc::DataRate::BitsPerSec(GetMinBitrateBps());
}

}  // namespace congestion_controller

RateControlInput::RateControlInput(
    BandwidthUsage bw_state,
    const absl::optional<webrtc::DataRate>& estimated_throughput)
    : bw_state(bw_state), estimated_throughput(estimated_throughput) {}

RateControlInput::~RateControlInput() = default;

}  // namespace webrtc

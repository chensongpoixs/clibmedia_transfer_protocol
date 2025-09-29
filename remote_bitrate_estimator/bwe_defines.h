
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



#ifndef _C_BWE_DEFINES_H_
#define _C_BWE_DEFINES_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/network_state_predictor.h"
#include "api/units/data_rate.h"

namespace libmedia_transfer_protocol {

namespace congestion_controller {
int GetMinBitrateBps();
webrtc::DataRate GetMinBitrate();
}  // namespace congestion_controller

static const int64_t kBitrateWindowMs = 1000;

extern const char kBweTypeHistogram[];

enum BweNames {
  kReceiverNoExtension = 0,
  kReceiverTOffset = 1,
  kReceiverAbsSendTime = 2,
  kSendSideTransportSeqNum = 3,
  kBweNamesMax = 4
};

struct RateControlInput {
  RateControlInput(BandwidthUsage bw_state,
                   const absl::optional<webrtc::DataRate>& estimated_throughput);
  ~RateControlInput();

  BandwidthUsage bw_state;
  absl::optional<webrtc::DataRate> estimated_throughput;
};
}  // namespace webrtc

#endif  // MODULES_REMOTE_BITRATE_ESTIMATOR_INCLUDE_BWE_DEFINES_H_

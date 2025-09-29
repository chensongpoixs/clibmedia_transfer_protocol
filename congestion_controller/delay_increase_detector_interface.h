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


				   ÑÓ³ÙÇ÷ÊÆ

 ******************************************************************************/
#ifndef _C_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_INCREASE_DETECTOR_INTERFACE_H_
#define _C_MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_INCREASE_DETECTOR_INTERFACE_H_

#include <stdint.h>

#include "libmedia_transfer_protocol/network_state_predictor.h"
#include "rtc_base/constructor_magic.h"

namespace libmedia_transfer_protocol {

class DelayIncreaseDetectorInterface {
 public:
  DelayIncreaseDetectorInterface() {}
  virtual ~DelayIncreaseDetectorInterface() {}

  // Update the detector with a new sample. The deltas should represent deltas
  // between timestamp groups as defined by the InterArrival class.
  virtual void Update(double recv_delta_ms,
                      double send_delta_ms,
                      int64_t send_time_ms,
                      int64_t arrival_time_ms,
                      size_t packet_size,
                      bool calculated_deltas) = 0;

  virtual BandwidthUsage State() const = 0;

  RTC_DISALLOW_COPY_AND_ASSIGN(DelayIncreaseDetectorInterface);
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_DELAY_INCREASE_DETECTOR_INTERFACE_H_

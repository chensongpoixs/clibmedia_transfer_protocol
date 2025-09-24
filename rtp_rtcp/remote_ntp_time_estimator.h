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
				   date:  2025-09-24



 ******************************************************************************/


#ifndef _C_MODULES_RTP_RTCP_INCLUDE_REMOTE_NTP_TIME_ESTIMATOR_H_
#define _C_MODULES_RTP_RTCP_INCLUDE_REMOTE_NTP_TIME_ESTIMATOR_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/numerics/moving_median_filter.h"
#include "system_wrappers/include/rtp_to_ntp_estimator.h"
#include "system_wrappers/include/clock.h"
namespace libmedia_transfer_protocol {

//class Clock;

// RemoteNtpTimeEstimator can be used to estimate a given RTP timestamp's NTP
// time in local timebase.
// Note that it needs to be trained with at least 2 RTCP SR (by calling
// `UpdateRtcpTimestamp`) before it can be used.
class RemoteNtpTimeEstimator {
 public:
  explicit RemoteNtpTimeEstimator(webrtc::Clock* clock);

  ~RemoteNtpTimeEstimator();

  // Updates the estimator with round trip time `rtt`, NTP seconds `ntp_secs`,
  // NTP fraction `ntp_frac` and RTP timestamp `rtp_timestamp`.
  bool UpdateRtcpTimestamp(int64_t rtt,
                           uint32_t ntp_secs,
                           uint32_t ntp_frac,
                           uint32_t rtp_timestamp);

  // Estimates the NTP timestamp in local timebase from `rtp_timestamp`.
  // Returns the NTP timestamp in ms when success. -1 if failed.
  int64_t Estimate(uint32_t rtp_timestamp);

  // Estimates the offset, in milliseconds, between the remote clock and the
  // local one. This is equal to local NTP clock - remote NTP clock.
  absl::optional<int64_t> EstimateRemoteToLocalClockOffsetMs();

 private:
	 webrtc::Clock* clock_;
	 webrtc::MovingMedianFilter<int64_t> ntp_clocks_offset_estimator_;
	 webrtc::RtpToNtpEstimator rtp_to_ntp_;
  int64_t last_timing_log_ms_;
  RTC_DISALLOW_COPY_AND_ASSIGN(RemoteNtpTimeEstimator);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_INCLUDE_REMOTE_NTP_TIME_ESTIMATOR_H_

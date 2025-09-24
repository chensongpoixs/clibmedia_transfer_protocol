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

#include "libmedia_transfer_protocol/rtp_rtcp/absolute_capture_time_sender.h"

#include <limits>

#include "libmedia_transfer_protocol/rtp_rtcp/absolute_capture_time_interpolator.h"
#include "system_wrappers/include/ntp_time.h"

namespace libmedia_transfer_protocol {
namespace {

constexpr webrtc::Timestamp kInvalidLastSendTime = webrtc::Timestamp::MinusInfinity();

}  // namespace

constexpr webrtc::TimeDelta AbsoluteCaptureTimeSender::kInterpolationMaxInterval;
constexpr webrtc::TimeDelta AbsoluteCaptureTimeSender::kInterpolationMaxError;

static_assert(
    AbsoluteCaptureTimeInterpolator::kInterpolationMaxInterval >=
        AbsoluteCaptureTimeSender::kInterpolationMaxInterval,
    "Receivers should be as willing to interpolate timestamps as senders.");

AbsoluteCaptureTimeSender::AbsoluteCaptureTimeSender(webrtc::Clock* clock)
    : clock_(clock), last_send_time_(kInvalidLastSendTime) {}

uint32_t AbsoluteCaptureTimeSender::GetSource(
    uint32_t ssrc,
    rtc::ArrayView<const uint32_t> csrcs) {
  return AbsoluteCaptureTimeInterpolator::GetSource(ssrc, csrcs);
}

absl::optional<AbsoluteCaptureTime> AbsoluteCaptureTimeSender::OnSendPacket(
    uint32_t source,
    uint32_t rtp_timestamp,
    uint32_t rtp_clock_frequency,
    uint64_t absolute_capture_timestamp,
    absl::optional<int64_t> estimated_capture_clock_offset) {
  const webrtc::Timestamp send_time = clock_->CurrentTime();

  webrtc::MutexLock lock(&mutex_);

  if (!ShouldSendExtension(send_time, source, rtp_timestamp,
                           rtp_clock_frequency, absolute_capture_timestamp,
                           estimated_capture_clock_offset)) {
    return absl::nullopt;
  }

  last_source_ = source;
  last_rtp_timestamp_ = rtp_timestamp;
  last_rtp_clock_frequency_ = rtp_clock_frequency;
  last_absolute_capture_timestamp_ = absolute_capture_timestamp;
  last_estimated_capture_clock_offset_ = estimated_capture_clock_offset;

  last_send_time_ = send_time;

  AbsoluteCaptureTime extension;
  extension.absolute_capture_timestamp = absolute_capture_timestamp;
  extension.estimated_capture_clock_offset = estimated_capture_clock_offset;
  return extension;
}

bool AbsoluteCaptureTimeSender::ShouldSendExtension(
	webrtc::Timestamp send_time,
    uint32_t source,
    uint32_t rtp_timestamp,
    uint32_t rtp_clock_frequency,
    uint64_t absolute_capture_timestamp,
    absl::optional<int64_t> estimated_capture_clock_offset) const {
  // Should if we've never sent anything before.
  if (last_send_time_ == kInvalidLastSendTime) {
    return true;
  }

  // Should if the last sent extension is too old.
  if ((send_time - last_send_time_) > kInterpolationMaxInterval) {
    return true;
  }

  // Should if the source has changed.
  if (last_source_ != source) {
    return true;
  }

  // Should if the RTP clock frequency has changed.
  if (last_rtp_clock_frequency_ != rtp_clock_frequency) {
    return true;
  }

  // Should if the RTP clock frequency is invalid.
  if (rtp_clock_frequency <= 0) {
    return true;
  }

  // Should if the estimated capture clock offset has changed.
  if (last_estimated_capture_clock_offset_ != estimated_capture_clock_offset) {
    return true;
  }

  // Should if interpolation would introduce too much error.
  const uint64_t interpolated_absolute_capture_timestamp =
      AbsoluteCaptureTimeInterpolator::InterpolateAbsoluteCaptureTimestamp(
          rtp_timestamp, rtp_clock_frequency, last_rtp_timestamp_,
          last_absolute_capture_timestamp_);
  const int64_t interpolation_error_ms = webrtc::UQ32x32ToInt64Ms(std::min(
      interpolated_absolute_capture_timestamp - absolute_capture_timestamp,
      absolute_capture_timestamp - interpolated_absolute_capture_timestamp));
  if (interpolation_error_ms > kInterpolationMaxError.ms()) {
    return true;
  }

  return false;
}

}  // namespace webrtc

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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_video_header.h"
#include "libmedia_codec/video_codec_type.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_h264.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_video_generic.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_vp8.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_vp9.h"
//
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packetizer_av1.h"
//#include "modules/video_coding/codecs/h264/include/h264_globals.h"
//#include "modules/video_coding/codecs/vp8/include/vp8_globals.h"
//#include "modules/video_coding/codecs/vp9/include/vp9_globals.h"


namespace libmedia_transfer_protocol {

class RtpPacketToSend;

class RtpPacketizer {
 public:
  struct PayloadSizeLimits {
    int max_payload_len = 1200;
    int first_packet_reduction_len = 0;
    int last_packet_reduction_len = 0;
    // Reduction len for packet that is first & last at the same time.
    int single_packet_reduction_len = 0;
  };

  // If type is not set, returns a raw packetizer.
  static std::unique_ptr<RtpPacketizer> Create(
	  absl::optional<libmedia_codec::VideoCodecType> type,
	  rtc::ArrayView<const uint8_t> payload,
	  PayloadSizeLimits limits,
	  // Codec-specific details.
	  const RTPVideoHeader& rtp_video_header);
 
 // static std::unique_ptr<RtpPacketizer> Create(absl::optional<libmedia_codec::VideoCodecType> type,
//	  rtc::ArrayView<const uint8_t> payload, PayloadSizeLimits limits, const RTPVideoHeader& rtp_video_header, int32_t u = 0);
  virtual ~RtpPacketizer() = default;

  // Returns number of remaining packets to produce by the packetizer.
  virtual size_t NumPackets() const = 0;

  // Get the next payload with payload header.
  // Write payload and set marker bit of the `packet`.
  // Returns true on success, false otherwise.
  virtual bool NextPacket(RtpPacketToSend* packet) = 0;

  // Split payload_len into sum of integers with respect to `limits`.
  // Returns empty vector on failure.
  static std::vector<int> SplitAboutEqually(int payload_len,
                                            const PayloadSizeLimits& limits);
};
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H_

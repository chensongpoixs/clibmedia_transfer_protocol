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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_

#include <bitset>
#include <cstdint>

#include "absl/container/inlined_vector.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "libmedia_transfer_protocol/rtp/dependency_descriptor.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_video_header.h"
#include "libmedia_codec/color_space.h"
#include "libmedia_codec/video_codec_type.h"
#include "libmedia_codec/video_content_type.h"
#include "libmedia_codec/video_frame_type.h"
#include "libmedia_codec/video_rotation.h"
#include "libmedia_codec/video_timing.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/codecs/vp8/include/vp8_globals.h"
#include "modules/video_coding/codecs/vp9/include/vp9_globals.h"

namespace libmedia_transfer_protocol {
// Details passed in the rtp payload for legacy generic rtp packetizer.
// TODO(bugs.webrtc.org/9772): Deprecate in favor of passing generic video
// details in an rtp header extension.
struct RTPVideoHeaderLegacyGeneric {
  uint16_t picture_id;
};

using RTPVideoTypeHeader = absl::variant<absl::monostate,
                                         webrtc::RTPVideoHeaderVP8,
	webrtc::RTPVideoHeaderVP9,
	webrtc::RTPVideoHeaderH264,
                                         RTPVideoHeaderLegacyGeneric>;

struct RTPVideoHeader {
  struct GenericDescriptorInfo {
    GenericDescriptorInfo();
    GenericDescriptorInfo(const GenericDescriptorInfo& other);
    ~GenericDescriptorInfo();

    int64_t frame_id = 0;
    int spatial_index = 0;
    int temporal_index = 0;
    absl::InlinedVector<DecodeTargetIndication, 10> decode_target_indications;
    absl::InlinedVector<int64_t, 5> dependencies;
    absl::InlinedVector<int, 4> chain_diffs;
    std::bitset<32> active_decode_targets = ~uint32_t{0};
  };

  RTPVideoHeader();
  RTPVideoHeader(const RTPVideoHeader& other);

  ~RTPVideoHeader();

  absl::optional<GenericDescriptorInfo> generic;

  libmedia_codec::VideoFrameType frame_type = libmedia_codec::VideoFrameType::kEmptyFrame;
  uint16_t width = 0;
  uint16_t height = 0;
  libmedia_codec::VideoRotation rotation = libmedia_codec::VideoRotation::kVideoRotation_0;
  libmedia_codec::VideoContentType content_type = libmedia_codec::VideoContentType::UNSPECIFIED;
  bool is_first_packet_in_frame = false;
  bool is_last_packet_in_frame = false;
  bool is_last_frame_in_picture = true;
  uint8_t simulcastIdx = 0;
  libmedia_codec::VideoCodecType codec = libmedia_codec::VideoCodecType::kVideoCodecGeneric;

  libmedia_codec::VideoPlayoutDelay playout_delay;
  libmedia_codec::VideoSendTiming video_timing;
  absl::optional<libmedia_codec::ColorSpace> color_space;
  // This field is meant for media quality testing purpose only. When enabled it
  // carries the webrtc::VideoFrame id field from the sender to the receiver.
  absl::optional<uint16_t> video_frame_tracking_id;
  RTPVideoTypeHeader video_type_header;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTP_VIDEO_HEADER_H_

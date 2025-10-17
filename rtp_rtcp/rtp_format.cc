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


#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format.h"

#include <memory>

#include "absl/types/variant.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_h264.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_video_generic.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_vp8.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_format_vp9.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packetizer_av1.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/codecs/vp8/include/vp8_globals.h"
#include "modules/video_coding/codecs/vp9/include/vp9_globals.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {
 
std::unique_ptr<RtpPacketizer> RtpPacketizer::Create(
    absl::optional<libmedia_codec::VideoCodecType> type,
    rtc::ArrayView<const uint8_t> payload,
    PayloadSizeLimits limits,
    // Codec-specific details.
    const RTPVideoHeader& rtp_video_header ) {


	if (!type) {
		// Use raw packetizer.
		return std::make_unique<RtpPacketizerGeneric>(payload, limits);
	}
	//return nullptr;

	switch (*type) {
		case libmedia_codec::kVideoCodecH264: {
			const auto& h264 =
				absl::get<webrtc::RTPVideoHeaderH264>(rtp_video_header.video_type_header);
			return std::make_unique<RtpPacketizerH264>(payload, limits,
				h264.packetization_mode);
		}
		case libmedia_codec::kVideoCodecVP8: {
			const auto& vp8 =
				absl::get<webrtc::RTPVideoHeaderVP8>(rtp_video_header.video_type_header);
			return std::make_unique<RtpPacketizerVp8>(payload, limits, vp8);
		}
		case libmedia_codec::kVideoCodecVP9: {
			const auto& vp9 =
				absl::get<webrtc::RTPVideoHeaderVP9>(rtp_video_header.video_type_header);
			return std::make_unique<RtpPacketizerVp9>(payload, limits, vp9);
		}
		case libmedia_codec::kVideoCodecAV1:
			return std::make_unique<RtpPacketizerAv1>(
				payload, limits, rtp_video_header.frame_type,
				rtp_video_header.is_last_frame_in_picture);
		default: {
			return std::make_unique<RtpPacketizerGeneric>(payload, limits,
				rtp_video_header);
		}
	}

  return  nullptr;
}
 
std::vector<int> RtpPacketizer::SplitAboutEqually(
    int payload_len,
    const PayloadSizeLimits& limits) {
  RTC_DCHECK_GT(payload_len, 0);
  // First or last packet larger than normal are unsupported.
  RTC_DCHECK_GE(limits.first_packet_reduction_len, 0);
  RTC_DCHECK_GE(limits.last_packet_reduction_len, 0);

  std::vector<int> result;
  // 容量足够
  if (limits.max_payload_len >=
      limits.single_packet_reduction_len + payload_len) {
    result.push_back(payload_len);
    return result;
  }
  // 容量太小
  if (limits.max_payload_len - limits.first_packet_reduction_len < 1 ||
      limits.max_payload_len - limits.last_packet_reduction_len < 1) {
    // Capacity is not enough to put a single byte into one of the packets.
    return result;
  }
  // First and last packet of the frame can be smaller. Pretend that it's
  // the same size, but we must write more payload to it.
  // Assume frame fits in single packet if packet has extra space for sum
  // of first and last packets reductions.
  // 需要均分的总字节数
  int total_bytes = payload_len + limits.first_packet_reduction_len +
                    limits.last_packet_reduction_len;
  // Integer divisions with rounding up.
  // 计算出我们应该分配多少个包合适，向上取整
  int num_packets_left =
      (total_bytes + limits.max_payload_len - 1) / limits.max_payload_len;
  if (num_packets_left == 1) {
    // Single packet is a special case handled above.
    num_packets_left = 2;
  }

  if (payload_len < num_packets_left) {
    // Edge case where limits force to have more packets than there are payload
    // bytes. This may happen when there is single byte of payload that can't be
    // put into single packet if
    // first_packet_reduction + last_packet_reduction >= max_payload_len.
    return result;
  }
  // 计算每一个分配的字节数
  int bytes_per_packet = total_bytes / num_packets_left;
  // 计算出有多少个包比其他包多1个字节
  int num_larger_packets = total_bytes % num_packets_left;
  int remaining_data = payload_len;

  result.reserve(num_packets_left);
  bool first_packet = true;
  while (remaining_data > 0) {
    // Last num_larger_packets are 1 byte wider than the rest. Increase
    // per-packet payload size when needed.
	  // 剩余的包需要多分配一个字节
		// total_bytes 5
		// 分配的个数3个包
		// 5 / 3 = 1, 1 2 2 11 3
		// 5 % 3 = 2
    if (num_packets_left == num_larger_packets)
      ++bytes_per_packet;
    int current_packet_bytes = bytes_per_packet;
	// 考虑第一个包的大小
    if (first_packet) {
      if (current_packet_bytes > limits.first_packet_reduction_len + 1)
        current_packet_bytes -= limits.first_packet_reduction_len;
      else
        current_packet_bytes = 1;
    }
	// 当剩余数据不足时，需要特殊考虑
    if (current_packet_bytes > remaining_data) {
      current_packet_bytes = remaining_data;
    }
    // This is not the last packet in the whole payload, but there's no data
    // left for the last packet. Leave at least one byte for the last packet.
	// 确保最后一个分组能够分到数据
    if (num_packets_left == 2 && current_packet_bytes == remaining_data) {
      --current_packet_bytes;
    }
    result.push_back(current_packet_bytes);

    remaining_data -= current_packet_bytes;
    --num_packets_left;
    first_packet = false;
  }

  return result;
}

}  // namespace webrtc

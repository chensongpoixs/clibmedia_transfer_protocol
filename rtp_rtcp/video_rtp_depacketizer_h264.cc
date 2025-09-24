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


#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer_h264.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "absl/base/macros.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "common_video/h264/h264_common.h"
#include "common_video/h264/pps_parser.h"
#include "common_video/h264/sps_parser.h"
#include "common_video/h264/sps_vui_rewriter.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
#include "libmedia_transfer_protocol/rtp_rtcp/video_rtp_depacketizer.h"
#include "rtc_base/checks.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/logging.h"

namespace libmedia_transfer_protocol {
namespace {

constexpr size_t kNalHeaderSize = 1;
constexpr size_t kFuAHeaderSize = 2;
constexpr size_t kLengthFieldSize = 2;
constexpr size_t kStapAHeaderSize = kNalHeaderSize + kLengthFieldSize;

// Bit masks for FU (A and B) indicators.
enum NalDefs : uint8_t { kFBit = 0x80, kNriMask = 0x60, kTypeMask = 0x1F };

// Bit masks for FU (A and B) headers.
enum FuDefs : uint8_t { kSBit = 0x80, kEBit = 0x40, kRBit = 0x20 };

// TODO(pbos): Avoid parsing this here as well as inside the jitter buffer.
bool ParseStapAStartOffsets(const uint8_t* nalu_ptr,
                            size_t length_remaining,
                            std::vector<size_t>* offsets) {
  size_t offset = 0;
  while (length_remaining > 0) {
    // Buffer doesn't contain room for additional nalu length.
    if (length_remaining < sizeof(uint16_t))
      return false;
    uint16_t nalu_size = ByteReader<uint16_t>::ReadBigEndian(nalu_ptr);
    nalu_ptr += sizeof(uint16_t);
    length_remaining -= sizeof(uint16_t);
    if (nalu_size > length_remaining)
      return false;
    nalu_ptr += nalu_size;
    length_remaining -= nalu_size;

    offsets->push_back(offset + kStapAHeaderSize);
    offset += kLengthFieldSize + nalu_size;
  }
  return true;
}

absl::optional<VideoRtpDepacketizer::ParsedRtpPayload> ProcessStapAOrSingleNalu(
    rtc::CopyOnWriteBuffer rtp_payload) {
  const uint8_t* const payload_data = rtp_payload.cdata();
  absl::optional<VideoRtpDepacketizer::ParsedRtpPayload> parsed_payload(
      absl::in_place);
  bool modified_buffer = false;
  parsed_payload->video_payload = rtp_payload;
  parsed_payload->video_header.width = 0;
  parsed_payload->video_header.height = 0;
  parsed_payload->video_header.codec = webrtc::kVideoCodecH264;
  parsed_payload->video_header.simulcastIdx = 0;
  parsed_payload->video_header.is_first_packet_in_frame = true;
  auto& h264_header = parsed_payload->video_header.video_type_header
                          .emplace<webrtc::RTPVideoHeaderH264>();

  const uint8_t* nalu_start = payload_data + kNalHeaderSize;
  const size_t nalu_length = rtp_payload.size() - kNalHeaderSize;
  uint8_t nal_type = payload_data[0] & kTypeMask;
  std::vector<size_t> nalu_start_offsets;
  if (nal_type == webrtc::H264::NaluType::kStapA) {
    // Skip the StapA header (StapA NAL type + length).
    if (rtp_payload.size() <= kStapAHeaderSize) {
      RTC_LOG(LS_ERROR) << "StapA header truncated.";
      return absl::nullopt;
    }

    if (!ParseStapAStartOffsets(nalu_start, nalu_length, &nalu_start_offsets)) {
      RTC_LOG(LS_ERROR) << "StapA packet with incorrect NALU packet lengths.";
      return absl::nullopt;
    }

    h264_header.packetization_type = webrtc::kH264StapA;
    nal_type = payload_data[kStapAHeaderSize] & kTypeMask;
  } else {
    h264_header.packetization_type = webrtc::kH264SingleNalu;
    nalu_start_offsets.push_back(0);
  }
  h264_header.nalu_type = nal_type;
  parsed_payload->video_header.frame_type = webrtc::VideoFrameType::kVideoFrameDelta;

  nalu_start_offsets.push_back(rtp_payload.size() +
                               kLengthFieldSize);  // End offset.
  for (size_t i = 0; i < nalu_start_offsets.size() - 1; ++i) {
    size_t start_offset = nalu_start_offsets[i];
    // End offset is actually start offset for next unit, excluding length field
    // so remove that from this units length.
    size_t end_offset = nalu_start_offsets[i + 1] - kLengthFieldSize;
    if (end_offset - start_offset < webrtc::H264::kNaluTypeSize) {
      RTC_LOG(LS_ERROR) << "STAP-A packet too short";
      return absl::nullopt;
    }

	webrtc::NaluInfo nalu;
    nalu.type = payload_data[start_offset] & kTypeMask;
    nalu.sps_id = -1;
    nalu.pps_id = -1;
    start_offset += webrtc::H264::kNaluTypeSize;

    switch (nalu.type) {
      case webrtc::H264::NaluType::kSps: {
        // Check if VUI is present in SPS and if it needs to be modified to
        // avoid
        // excessive decoder latency.

        // Copy any previous data first (likely just the first header).
        rtc::Buffer output_buffer;
        if (start_offset)
          output_buffer.AppendData(payload_data, start_offset);

        absl::optional<webrtc::SpsParser::SpsState> sps;

		webrtc::SpsVuiRewriter::ParseResult result = webrtc::SpsVuiRewriter::ParseAndRewriteSps(
            &payload_data[start_offset], end_offset - start_offset, &sps,
            nullptr, &output_buffer, webrtc::SpsVuiRewriter::Direction::kIncoming);

        if (result == webrtc::SpsVuiRewriter::ParseResult::kVuiRewritten) {
          if (modified_buffer) {
            RTC_LOG(LS_WARNING)
                << "More than one H264 SPS NAL units needing "
                   "rewriting found within a single STAP-A packet. "
                   "Keeping the first and rewriting the last.";
          }

          // Rewrite length field to new SPS size.
          if (h264_header.packetization_type == webrtc::kH264StapA) {
            size_t length_field_offset =
                start_offset - (webrtc::H264::kNaluTypeSize + kLengthFieldSize);
            // Stap-A Length includes payload data and type header.
            size_t rewritten_size =
                output_buffer.size() - start_offset + webrtc::H264::kNaluTypeSize;
            ByteWriter<uint16_t>::WriteBigEndian(
                &output_buffer[length_field_offset], rewritten_size);
          }

          parsed_payload->video_payload.SetData(output_buffer.data(),
                                                output_buffer.size());
          // Append rest of packet.
          parsed_payload->video_payload.AppendData(
              &payload_data[end_offset],
              nalu_length + kNalHeaderSize - end_offset);

          modified_buffer = true;
        }

        if (sps) {
          parsed_payload->video_header.width = sps->width;
          parsed_payload->video_header.height = sps->height;
          nalu.sps_id = sps->id;
        } else {
          RTC_LOG(LS_WARNING) << "Failed to parse SPS id from SPS slice.";
        }
        parsed_payload->video_header.frame_type =
			webrtc::VideoFrameType::kVideoFrameKey;
        break;
      }
      case webrtc::H264::NaluType::kPps: {
        uint32_t pps_id;
        uint32_t sps_id;
        if (webrtc::PpsParser::ParsePpsIds(&payload_data[start_offset],
                                   end_offset - start_offset, &pps_id,
                                   &sps_id)) {
          nalu.pps_id = pps_id;
          nalu.sps_id = sps_id;
        } else {
          RTC_LOG(LS_WARNING)
              << "Failed to parse PPS id and SPS id from PPS slice.";
        }
        break;
      }
      case webrtc::H264::NaluType::kIdr:
        parsed_payload->video_header.frame_type =
			webrtc::VideoFrameType::kVideoFrameKey;
        ABSL_FALLTHROUGH_INTENDED;
      case webrtc::H264::NaluType::kSlice: {
        absl::optional<uint32_t> pps_id = webrtc::PpsParser::ParsePpsIdFromSlice(
            &payload_data[start_offset], end_offset - start_offset);
        if (pps_id) {
          nalu.pps_id = *pps_id;
        } else {
          RTC_LOG(LS_WARNING) << "Failed to parse PPS id from slice of type: "
                              << static_cast<int>(nalu.type);
        }
        break;
      }
      // Slices below don't contain SPS or PPS ids.
      case webrtc::H264::NaluType::kAud:
      case webrtc::H264::NaluType::kEndOfSequence:
      case webrtc::H264::NaluType::kEndOfStream:
      case webrtc::H264::NaluType::kFiller:
      case webrtc::H264::NaluType::kSei:
        break;
      case webrtc::H264::NaluType::kStapA:
      case webrtc::H264::NaluType::kFuA:
        RTC_LOG(LS_WARNING) << "Unexpected STAP-A or FU-A received.";
        return absl::nullopt;
    }

    if (h264_header.nalus_length == webrtc::kMaxNalusPerPacket) {
      RTC_LOG(LS_WARNING)
          << "Received packet containing more than " << webrtc::kMaxNalusPerPacket
          << " NAL units. Will not keep track sps and pps ids for all of them.";
    } else {
      h264_header.nalus[h264_header.nalus_length++] = nalu;
    }
  }

  return parsed_payload;
}

absl::optional<VideoRtpDepacketizer::ParsedRtpPayload> ParseFuaNalu(
    rtc::CopyOnWriteBuffer rtp_payload) {
  if (rtp_payload.size() < kFuAHeaderSize) {
    RTC_LOG(LS_ERROR) << "FU-A NAL units truncated.";
    return absl::nullopt;
  }
  absl::optional<VideoRtpDepacketizer::ParsedRtpPayload> parsed_payload(
      absl::in_place);
  uint8_t fnri = rtp_payload.cdata()[0] & (kFBit | kNriMask);
  uint8_t original_nal_type = rtp_payload.cdata()[1] & kTypeMask;
  bool first_fragment = (rtp_payload.cdata()[1] & kSBit) > 0;
  webrtc::NaluInfo nalu;
  nalu.type = original_nal_type;
  nalu.sps_id = -1;
  nalu.pps_id = -1;
  if (first_fragment) {
    absl::optional<uint32_t> pps_id =
		webrtc::PpsParser::ParsePpsIdFromSlice(rtp_payload.cdata() + 2 * kNalHeaderSize,
                                       rtp_payload.size() - 2 * kNalHeaderSize);
    if (pps_id) {
      nalu.pps_id = *pps_id;
    } else {
      RTC_LOG(LS_WARNING)
          << "Failed to parse PPS from first fragment of FU-A NAL "
             "unit with original type: "
          << static_cast<int>(nalu.type);
    }
    uint8_t original_nal_header = fnri | original_nal_type;
    rtp_payload =
        rtp_payload.Slice(kNalHeaderSize, rtp_payload.size() - kNalHeaderSize);
    rtp_payload.MutableData()[0] = original_nal_header;
    parsed_payload->video_payload = std::move(rtp_payload);
  } else {
    parsed_payload->video_payload =
        rtp_payload.Slice(kFuAHeaderSize, rtp_payload.size() - kFuAHeaderSize);
  }

  if (original_nal_type == webrtc::H264::NaluType::kIdr) {
    parsed_payload->video_header.frame_type = webrtc::VideoFrameType::kVideoFrameKey;
  } else {
    parsed_payload->video_header.frame_type = webrtc::VideoFrameType::kVideoFrameDelta;
  }
  parsed_payload->video_header.width = 0;
  parsed_payload->video_header.height = 0;
  parsed_payload->video_header.codec = webrtc::kVideoCodecH264;
  parsed_payload->video_header.simulcastIdx = 0;
  parsed_payload->video_header.is_first_packet_in_frame = first_fragment;
  auto& h264_header = parsed_payload->video_header.video_type_header
                          .emplace<webrtc::RTPVideoHeaderH264>();
  h264_header.packetization_type = webrtc::kH264FuA;
  h264_header.nalu_type = original_nal_type;
  if (first_fragment) {
    h264_header.nalus[h264_header.nalus_length] = nalu;
    h264_header.nalus_length = 1;
  }
  return parsed_payload;
}

}  // namespace

absl::optional<VideoRtpDepacketizer::ParsedRtpPayload>
VideoRtpDepacketizerH264::Parse(rtc::CopyOnWriteBuffer rtp_payload) {
  if (rtp_payload.size() == 0) {
    RTC_LOG(LS_ERROR) << "Empty payload.";
    return absl::nullopt;
  }

  uint8_t nal_type = rtp_payload.cdata()[0] & kTypeMask;

  if (nal_type == webrtc::H264::NaluType::kFuA) {
    // Fragmented NAL units (FU-A).
    return ParseFuaNalu(std::move(rtp_payload));
  } else {
    // We handle STAP-A and single NALU's the same way here. The jitter buffer
    // will depacketize the STAP-A into NAL units later.
    return ProcessStapAOrSingleNalu(std::move(rtp_payload));
  }
}

}  // namespace webrtc

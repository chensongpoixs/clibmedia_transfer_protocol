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
				   date:  2025-10-06



 ******************************************************************************/


#ifndef _C_API_VIDEO_RTP_VIDEO_FRAME_ASSEMBLER_H_
#define _C_API_VIDEO_RTP_VIDEO_FRAME_ASSEMBLER_H_

#include <cstdint>
#include <memory>

#include "absl/container/inlined_vector.h"
#include "libmedia_codec/encoded_frame.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"

namespace libmedia_transfer_protocol {
// The RtpVideoFrameAssembler takes RtpPacketReceived and assembles them into
// complete frames. A frame is considered complete when all packets of the frame
// has been received, the bitstream data has successfully extracted, an ID has
// been assigned, and all dependencies are known. Frame IDs are strictly
// monotonic in decode order, dependencies are expressed as frame IDs.
class RtpVideoFrameAssembler {
 public:
  // FrameVector is just a vector-like type of std::unique_ptr<EncodedFrame>.
  // The vector type may change without notice.
  using FrameVector = absl::InlinedVector<std::unique_ptr<libmedia_codec::EncodedFrame>, 3>;
  enum PayloadFormat { kRaw, kH264, kVp8, kVp9, kAv1, kGeneric };

  explicit RtpVideoFrameAssembler(PayloadFormat payload_format);
  RtpVideoFrameAssembler(const RtpVideoFrameAssembler& other) = delete;
  RtpVideoFrameAssembler& operator=(const RtpVideoFrameAssembler& other) =
      delete;
  ~RtpVideoFrameAssembler();

  // Typically when a packet is inserted zero or one frame is completed. In the
  // case of RTP packets being inserted out of order then sometime multiple
  // frames could be completed from a single packet, hence the 'FrameVector'
  // return type.
  FrameVector InsertPacket(const RtpPacketReceived& packet);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace webrtc

#endif  // API_VIDEO_RTP_VIDEO_FRAME_ASSEMBLER_H_

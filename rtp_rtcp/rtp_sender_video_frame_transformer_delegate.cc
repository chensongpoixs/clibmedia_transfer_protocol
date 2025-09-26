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

#include "libmedia_transfer_protocol/rtp_rtcp/rtp_sender_video_frame_transformer_delegate.h"

#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_descriptor_authentication.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_sender_video.h"
#include "rtc_base/task_utils/to_queued_task.h"

namespace libmedia_transfer_protocol {
namespace {

class TransformableVideoSenderFrame : public TransformableVideoFrameInterface {
 public:
  TransformableVideoSenderFrame(
      const libmedia_codec::EncodedImage& encoded_image,
      const RTPVideoHeader& video_header,
      int payload_type,
      absl::optional<libmedia_codec::VideoCodecType> codec_type,
      uint32_t rtp_timestamp,
      absl::optional<int64_t> expected_retransmission_time_ms,
      uint32_t ssrc)
      : encoded_data_(encoded_image.GetEncodedData()),
        header_(video_header),
        //metadata_(header_),
        frame_type_(encoded_image._frameType),
        payload_type_(payload_type),
        codec_type_(codec_type),
        timestamp_(rtp_timestamp),
        capture_time_ms_(encoded_image.capture_time_ms_),
        expected_retransmission_time_ms_(expected_retransmission_time_ms),
        ssrc_(ssrc) {
    RTC_DCHECK_GE(payload_type_, 0);
    RTC_DCHECK_LE(payload_type_, 127);
  }

  ~TransformableVideoSenderFrame() override = default;

  // Implements TransformableVideoFrameInterface.
  rtc::ArrayView<const uint8_t> GetData() const override {
    return *encoded_data_;
  }

  void SetData(rtc::ArrayView<const uint8_t> data) override {
    encoded_data_ = libmedia_codec::EncodedImageBuffer::Create(data.data(), data.size());
  }

  uint32_t GetTimestamp() const override { return timestamp_; }
  uint32_t GetSsrc() const override { return ssrc_; }

  bool IsKeyFrame() const override {
    return frame_type_ == libmedia_codec::VideoFrameType::kVideoFrameKey;
  }

  std::vector<uint8_t> GetAdditionalData() const override {
    return RtpDescriptorAuthentication(header_);
  }

 // const VideoFrameMetadata& GetMetadata() const override { return metadata_; }

  const RTPVideoHeader& GetHeader() const { return header_; }
  uint8_t GetPayloadType() const override { return payload_type_; }
  absl::optional<libmedia_codec::VideoCodecType> GetCodecType() const { return codec_type_; }
  int64_t GetCaptureTimeMs() const { return capture_time_ms_; }

  const absl::optional<int64_t>& GetExpectedRetransmissionTimeMs() const {
    return expected_retransmission_time_ms_;
  }

  Direction GetDirection() const override { return Direction::kSender; }

 private:
  rtc::scoped_refptr<libmedia_codec::EncodedImageBufferInterface> encoded_data_;
  const RTPVideoHeader header_;
 // const webrtc::VideoFrameMetadata metadata_;
  const libmedia_codec::VideoFrameType frame_type_;
  const uint8_t payload_type_;
  const absl::optional<libmedia_codec::VideoCodecType> codec_type_ = absl::nullopt;
  const uint32_t timestamp_;
  const int64_t capture_time_ms_;
  const absl::optional<int64_t> expected_retransmission_time_ms_;
  const uint32_t ssrc_;
};
}  // namespace

RTPSenderVideoFrameTransformerDelegate::RTPSenderVideoFrameTransformerDelegate(
    RTPSenderVideo* sender,
    rtc::scoped_refptr<FrameTransformerInterface> frame_transformer,
    uint32_t ssrc,
	webrtc::TaskQueueBase* send_transport_queue)
    : sender_(sender),
      frame_transformer_(std::move(frame_transformer)),
      ssrc_(ssrc),
      send_transport_queue_(send_transport_queue) {}

void RTPSenderVideoFrameTransformerDelegate::Init() {
  frame_transformer_->RegisterTransformedFrameSinkCallback(
      rtc::scoped_refptr<TransformedFrameCallback>(this), ssrc_);
}

bool RTPSenderVideoFrameTransformerDelegate::TransformFrame(
    int payload_type,
    absl::optional<libmedia_codec::VideoCodecType> codec_type,
    uint32_t rtp_timestamp,
    const libmedia_codec::EncodedImage& encoded_image,
    RTPVideoHeader video_header,
    absl::optional<int64_t> expected_retransmission_time_ms) {
  if (!encoder_queue_) {
    // Save the current task queue to post the transformed frame for sending
    // once it is transformed. When there is no current task queue, i.e.
    // encoding is done on an external thread (for example in the case of
    // hardware encoders), use the send transport queue instead.
	  webrtc::TaskQueueBase* current = webrtc::TaskQueueBase::Current();
    encoder_queue_ = current ? current : send_transport_queue_;
  }
  frame_transformer_->Transform(std::make_unique<TransformableVideoSenderFrame>(
      encoded_image, video_header, payload_type, codec_type, rtp_timestamp,
      expected_retransmission_time_ms, ssrc_));
  return true;
}

void RTPSenderVideoFrameTransformerDelegate::OnTransformedFrame(
    std::unique_ptr<TransformableFrameInterface> frame) {
	webrtc::MutexLock lock(&sender_lock_);

  // The encoder queue normally gets destroyed after the sender;
  // however, it might still be null by the time a previously queued frame
  // arrives.
  if (!sender_ || !encoder_queue_)
    return;
  rtc::scoped_refptr<RTPSenderVideoFrameTransformerDelegate> delegate = this;
  encoder_queue_->PostTask(webrtc::ToQueuedTask(
      [delegate = std::move(delegate), frame = std::move(frame)]() mutable {
        delegate->SendVideo(std::move(frame));
      }));
}

void RTPSenderVideoFrameTransformerDelegate::SendVideo(
    std::unique_ptr<TransformableFrameInterface> transformed_frame) const {
  RTC_CHECK(encoder_queue_->IsCurrent());
  RTC_CHECK_EQ(transformed_frame->GetDirection(),
               TransformableFrameInterface::Direction::kSender);
  webrtc::MutexLock lock(&sender_lock_);
  if (!sender_)
    return;
  auto* transformed_video_frame =
      static_cast<TransformableVideoSenderFrame*>(transformed_frame.get());
  sender_->SendVideo(
      transformed_video_frame->GetPayloadType(),
      transformed_video_frame->GetCodecType(),
      transformed_video_frame->GetTimestamp(),
      transformed_video_frame->GetCaptureTimeMs(),
      transformed_video_frame->GetData(),
      transformed_video_frame->GetHeader(),
      transformed_video_frame->GetExpectedRetransmissionTimeMs());
}

void RTPSenderVideoFrameTransformerDelegate::SetVideoStructureUnderLock(
    const FrameDependencyStructure* video_structure) {
	webrtc::MutexLock lock(&sender_lock_);
  RTC_CHECK(sender_);
  sender_->SetVideoStructureAfterTransformation(video_structure);
}

void RTPSenderVideoFrameTransformerDelegate::SetVideoLayersAllocationUnderLock(
	libmedia_codec::VideoLayersAllocation allocation) {
	webrtc::MutexLock lock(&sender_lock_);
  RTC_CHECK(sender_);
  sender_->SetVideoLayersAllocationAfterTransformation(std::move(allocation));
}

void RTPSenderVideoFrameTransformerDelegate::Reset() {
  frame_transformer_->UnregisterTransformedFrameSinkCallback(ssrc_);
  frame_transformer_ = nullptr;
  {
	  webrtc::MutexLock lock(&sender_lock_);
    sender_ = nullptr;
  }
}
}  // namespace webrtc

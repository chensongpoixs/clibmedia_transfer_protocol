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



#include "libmtp/rtp_receiver_interface.h"

namespace libmtp {

std::vector<std::string> RtpReceiverInterface::stream_ids() const {
  return {};
}

std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>
RtpReceiverInterface::streams() const {
  return {};
}

std::vector<webrtc::RtpSource> RtpReceiverInterface::GetSources() const {
  return {};
}

void RtpReceiverInterface::SetFrameDecryptor(
    rtc::scoped_refptr<webrtc::FrameDecryptorInterface> frame_decryptor) {}

rtc::scoped_refptr<webrtc::FrameDecryptorInterface>
RtpReceiverInterface::GetFrameDecryptor() const {
  return nullptr;
}

rtc::scoped_refptr<libice::DtlsTransportInterface>
RtpReceiverInterface::dtls_transport() const {
  return nullptr;
}

void RtpReceiverInterface::SetDepacketizerToDecoderFrameTransformer(
    rtc::scoped_refptr<webrtc::FrameTransformerInterface> frame_transformer) {}

}  // namespace webrtc

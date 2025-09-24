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



#include "libmedia_transfer_protocol/rtp_sender_interface.h"

namespace libmedia_transfer_protocol {

void RtpSenderInterface::SetFrameEncryptor(
    rtc::scoped_refptr<FrameEncryptorInterface> frame_encryptor) {}

rtc::scoped_refptr<FrameEncryptorInterface>
RtpSenderInterface::GetFrameEncryptor() const {
  return nullptr;
}

std::vector<libmedia_transfer_protocol::RtpEncodingParameters> RtpSenderInterface::init_send_encodings()
    const {
  return {};
}

rtc::scoped_refptr<libice::DtlsTransportInterface> RtpSenderInterface::dtls_transport()
    const {
  return nullptr;
}

void RtpSenderInterface::SetEncoderToPacketizerFrameTransformer(
    rtc::scoped_refptr<libmedia_transfer_protocol::FrameTransformerInterface> frame_transformer) {}

}  // namespace webrtc

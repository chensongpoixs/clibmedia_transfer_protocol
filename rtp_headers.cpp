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



#include "libmedia_transfer_protocol/rtp_headers.h"

namespace libmedia_transfer_protocol {

RTPHeaderExtension::RTPHeaderExtension()
    : hasTransmissionTimeOffset(false),
      transmissionTimeOffset(0),
      hasAbsoluteSendTime(false),
      absoluteSendTime(0),
      hasTransportSequenceNumber(false),
      transportSequenceNumber(0),
      hasAudioLevel(false),
      voiceActivity(false),
      audioLevel(0),
      hasVideoRotation(false),
      videoRotation(libmedia_codec::kVideoRotation_0),
      hasVideoContentType(false),
      videoContentType(libmedia_codec::VideoContentType::UNSPECIFIED),
      has_video_timing(false) {}

RTPHeaderExtension::RTPHeaderExtension(const RTPHeaderExtension& other) =
    default;

RTPHeaderExtension& RTPHeaderExtension::operator=(
    const RTPHeaderExtension& other) = default;

RTPHeader::RTPHeader()
    : markerBit(false),
      payloadType(0),
      sequenceNumber(0),
      timestamp(0),
      ssrc(0),
      numCSRCs(0),
      arrOfCSRCs(),
      paddingLength(0),
      headerLength(0),
      payload_type_frequency(0),
      extension() {}

RTPHeader::RTPHeader(const RTPHeader& other) = default;

RTPHeader& RTPHeader::operator=(const RTPHeader& other) = default;

}  // namespace webrtc

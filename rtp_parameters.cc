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

#include "libmtp/rtp_parameters.h"

#include <algorithm>
#include <string>
#include <utility>

#include "api/array_view.h"
#include "rtc_base/strings/string_builder.h"

namespace libmtp {

 
const double kDefaultBitratePriority = 1.0;
 
std::string RtpExtension::ToString() const {
  char buf[256];
  rtc::SimpleStringBuilder sb(buf);
  sb << "{uri: " << uri;
  sb << ", id: " << id;
  if (encrypt) {
    sb << ", encrypt";
  }
  sb << '}';
  return sb.str();
}

constexpr char RtpExtension::kEncryptHeaderExtensionsUri[];
constexpr char RtpExtension::kAudioLevelUri[];
constexpr char RtpExtension::kTimestampOffsetUri[];
constexpr char RtpExtension::kAbsSendTimeUri[];
constexpr char RtpExtension::kAbsoluteCaptureTimeUri[];
constexpr char RtpExtension::kVideoRotationUri[];
constexpr char RtpExtension::kVideoContentTypeUri[];
constexpr char RtpExtension::kVideoTimingUri[];
constexpr char RtpExtension::kGenericFrameDescriptorUri00[];
constexpr char RtpExtension::kDependencyDescriptorUri[];
constexpr char RtpExtension::kVideoLayersAllocationUri[];
constexpr char RtpExtension::kTransportSequenceNumberUri[];
constexpr char RtpExtension::kTransportSequenceNumberV2Uri[];
constexpr char RtpExtension::kPlayoutDelayUri[];
constexpr char RtpExtension::kColorSpaceUri[];
constexpr char RtpExtension::kMidUri[];
constexpr char RtpExtension::kRidUri[];
constexpr char RtpExtension::kRepairedRidUri[];
constexpr char RtpExtension::kVideoFrameTrackingIdUri[];
constexpr char RtpExtension::kCsrcAudioLevelsUri[];

constexpr int RtpExtension::kMinId;
constexpr int RtpExtension::kMaxId;
constexpr int RtpExtension::kMaxValueSize;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxId;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxValueSize;
 
}  // namespace webrtc

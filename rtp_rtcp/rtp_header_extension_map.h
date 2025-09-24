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

#ifndef _C_MODULES_RTP_RTCP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_
#define _C_MODULES_RTP_RTCP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_

#include <stdint.h>

#include <string>

#include "absl/strings/string_view.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_parameters.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {

class RtpHeaderExtensionMap {
 public:
  static constexpr RTPExtensionType kInvalidType = kRtpExtensionNone;
  static constexpr int kInvalidId = 0;

  RtpHeaderExtensionMap();
  explicit RtpHeaderExtensionMap(bool extmap_allow_mixed);
  explicit RtpHeaderExtensionMap(rtc::ArrayView<const RtpExtension> extensions);

  void Reset(rtc::ArrayView<const RtpExtension> extensions);

  template <typename Extension>
  bool Register(int id) {
    return Register(id, Extension::kId, Extension::Uri());
  }
  bool RegisterByType(int id, RTPExtensionType type);
  bool RegisterByUri(int id, absl::string_view uri);

  bool IsRegistered(RTPExtensionType type) const {
    return GetId(type) != kInvalidId;
  }
  // Return kInvalidType if not found.
  RTPExtensionType GetType(int id) const;
  // Return kInvalidId if not found.
  uint8_t GetId(RTPExtensionType type) const {
    RTC_DCHECK_GT(type, kRtpExtensionNone);
    RTC_DCHECK_LT(type, kRtpExtensionNumberOfExtensions);
    return ids_[type];
  }

  int32_t Deregister(RTPExtensionType type);
  void Deregister(absl::string_view uri);

  // Corresponds to the SDP attribute extmap-allow-mixed, see RFC8285.
  // Set to true if it's allowed to mix one- and two-byte RTP header extensions
  // in the same stream.
  bool ExtmapAllowMixed() const { return extmap_allow_mixed_; }
  void SetExtmapAllowMixed(bool extmap_allow_mixed) {
    extmap_allow_mixed_ = extmap_allow_mixed;
  }

 private:
  bool Register(int id, RTPExtensionType type, absl::string_view uri);

  uint8_t ids_[kRtpExtensionNumberOfExtensions];
  bool extmap_allow_mixed_;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_

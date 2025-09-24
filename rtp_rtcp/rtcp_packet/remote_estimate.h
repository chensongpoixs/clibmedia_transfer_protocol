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
				   date:  2025-09-24



 ******************************************************************************/

#ifndef _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_REMOTE_ESTIMATE_H_
#define _C_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_REMOTE_ESTIMATE_H_

#include <memory>
#include <vector>

#include "libice/network_types.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/app.h"

namespace libmedia_transfer_protocol {
namespace rtcp {

class CommonHeader;
class RemoteEstimateSerializer {
 public:
  virtual bool Parse(rtc::ArrayView<const uint8_t> src,
                     libice::NetworkStateEstimate* target) const = 0;
  virtual rtc::Buffer Serialize(const libice::NetworkStateEstimate& src) const = 0;
  virtual ~RemoteEstimateSerializer() = default;
};

// Using a static global implementation to avoid incurring initialization
// overhead of the serializer every time RemoteEstimate is created.
const RemoteEstimateSerializer* GetRemoteEstimateSerializer();

// The RemoteEstimate packet provides network estimation results from the
// receive side. This functionality is experimental and subject to change
// without notice.
class RemoteEstimate : public App {
 public:
  RemoteEstimate();
  explicit RemoteEstimate(App&& app);
  // Note, sub type must be unique among all app messages with "goog" name.
  static constexpr uint8_t kSubType = 13;
  static constexpr uint32_t kName = NameToInt("goog");
  static webrtc::TimeDelta GetTimestampPeriod();

  bool ParseData();
  void SetEstimate(libice::NetworkStateEstimate estimate);
  libice::NetworkStateEstimate estimate() const { return estimate_; }

 private:
  libice::NetworkStateEstimate estimate_;
  const RemoteEstimateSerializer* const serializer_;
};

}  // namespace rtcp
}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_REMOTE_ESTIMATE_H_

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


#include "libmedia_transfer_protocol/rtp_rtcp/rtp_dependency_descriptor_extension.h"

#include <bitset>
#include <cstdint>

#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp/dependency_descriptor.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_dependency_descriptor_reader.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_dependency_descriptor_writer.h"
#include "rtc_base/numerics/divide_round.h"

namespace libmedia_transfer_protocol {

constexpr RTPExtensionType RtpDependencyDescriptorExtension::kId;
constexpr std::bitset<32> RtpDependencyDescriptorExtension::kAllChainsAreActive;

bool RtpDependencyDescriptorExtension::Parse(
    rtc::ArrayView<const uint8_t> data,
    const FrameDependencyStructure* structure,
    DependencyDescriptor* descriptor) {
  RtpDependencyDescriptorReader reader(data, structure, descriptor);
  return reader.ParseSuccessful();
}

size_t RtpDependencyDescriptorExtension::ValueSize(
    const FrameDependencyStructure& structure,
    std::bitset<32> active_chains,
    const DependencyDescriptor& descriptor) {
  RtpDependencyDescriptorWriter writer(/*data=*/{}, structure, active_chains,
                                       descriptor);
  return webrtc::DivideRoundUp(writer.ValueSizeBits(), 8);
}

bool RtpDependencyDescriptorExtension::Write(
    rtc::ArrayView<uint8_t> data,
    const FrameDependencyStructure& structure,
    std::bitset<32> active_chains,
    const DependencyDescriptor& descriptor) {
  RtpDependencyDescriptorWriter writer(data, structure, active_chains,
                                       descriptor);
  return writer.Write();
}

}  // namespace webrtc

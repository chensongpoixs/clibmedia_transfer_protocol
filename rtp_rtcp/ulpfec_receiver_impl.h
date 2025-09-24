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

#ifndef _C_MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_H_
#define _C_MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "api/sequence_checker.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_header_extension_map.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_rtcp_defines.h"
#include "libmedia_transfer_protocol/rtp_rtcp//ulpfec_receiver.h"
#include "libmedia_transfer_protocol/rtp_rtcp/forward_error_correction.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
#include "rtc_base/system/no_unique_address.h"

namespace libmedia_transfer_protocol {

class UlpfecReceiverImpl : public UlpfecReceiver {
 public:
  explicit UlpfecReceiverImpl(uint32_t ssrc,
                              RecoveredPacketReceiver* callback,
                              rtc::ArrayView<const RtpExtension> extensions);
  ~UlpfecReceiverImpl() override;

  bool AddReceivedRedPacket(const RtpPacketReceived& rtp_packet,
                            uint8_t ulpfec_payload_type) override;

  int32_t ProcessReceivedFec() override;

  FecPacketCounter GetPacketCounter() const override;

  void SetRtpExtensions(rtc::ArrayView<const RtpExtension> extensions) override;

 private:
  const uint32_t ssrc_;
  RtpHeaderExtensionMap extensions_ RTC_GUARDED_BY(&sequence_checker_);

  RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker sequence_checker_;
  RecoveredPacketReceiver* const recovered_packet_callback_;
  const std::unique_ptr<ForwardErrorCorrection> fec_;
  // TODO(nisse): The AddReceivedRedPacket method adds one or two packets to
  // this list at a time, after which it is emptied by ProcessReceivedFec. It
  // will make things simpler to merge AddReceivedRedPacket and
  // ProcessReceivedFec into a single method, and we can then delete this list.
  std::vector<std::unique_ptr<ForwardErrorCorrection::ReceivedPacket>>
      received_packets_ RTC_GUARDED_BY(&sequence_checker_);
  ForwardErrorCorrection::RecoveredPacketList recovered_packets_
      RTC_GUARDED_BY(&sequence_checker_);
  FecPacketCounter packet_counter_ RTC_GUARDED_BY(&sequence_checker_);
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_ULPFEC_RECEIVER_IMPL_H_

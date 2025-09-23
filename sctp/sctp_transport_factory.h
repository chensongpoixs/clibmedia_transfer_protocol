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
				   date:  2025-09-23



 ******************************************************************************/

#ifndef _C_MEDIA_SCTP_SCTP_TRANSPORT_FACTORY_H_
#define _C_MEDIA_SCTP_SCTP_TRANSPORT_FACTORY_H_

#include <memory>

#include "libmtp/sctp/sctp_transport_factory_interface.h"
#include "libmtp/sctp/sctp_transport_internal.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/thread.h"

namespace libmtp {

class SctpTransportFactory : public libmtp::SctpTransportFactoryInterface {
 public:
  explicit SctpTransportFactory(rtc::Thread* network_thread);

  std::unique_ptr<SctpTransportInternal> CreateSctpTransport(
      libice::PacketTransportInternal* transport) override;

 private:
  rtc::Thread* network_thread_;
  webrtc::FieldTrialFlag use_dcsctp_;
};

}  // namespace cricket

#endif  // MEDIA_SCTP_SCTP_TRANSPORT_FACTORY_H__

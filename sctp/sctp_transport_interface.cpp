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


#include "libmedia_transfer_protocol/sctp/sctp_transport_interface.h"

#include <utility>

namespace libmedia_transfer_protocol {

SctpTransportInformation::SctpTransportInformation(SctpTransportState state)
    : state_(state) {}

SctpTransportInformation::SctpTransportInformation(
    SctpTransportState state,
    rtc::scoped_refptr<libice::DtlsTransportInterface> dtls_transport,
    absl::optional<double> max_message_size,
    absl::optional<int> max_channels)
    : state_(state),
      dtls_transport_(std::move(dtls_transport)),
      max_message_size_(max_message_size),
      max_channels_(max_channels) {}

SctpTransportInformation::~SctpTransportInformation() {}

}  // namespace webrtc

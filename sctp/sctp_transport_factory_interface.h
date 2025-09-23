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


#ifndef _C_API_TRANSPORT_SCTP_TRANSPORT_FACTORY_INTERFACE_H_
#define _C_API_TRANSPORT_SCTP_TRANSPORT_FACTORY_INTERFACE_H_

#include <memory>

// These classes are not part of the API, and are treated as opaque pointers.
namespace libmtp {
class SctpTransportInternal;
}  // namespace cricket

namespace libice {
class PacketTransportInternal;
}  // namespace rtc

namespace libmtp {

// Factory class which can be used to allow fake SctpTransports to be injected
// for testing. An application is not intended to implement this interface nor
// 'cricket::SctpTransportInternal' because SctpTransportInternal is not
// guaranteed to remain stable in future WebRTC versions.
class SctpTransportFactoryInterface {
 public:
  virtual ~SctpTransportFactoryInterface() = default;

  // Create an SCTP transport using `channel` for the underlying transport.
  virtual std::unique_ptr<libmtp::SctpTransportInternal> CreateSctpTransport(
      libice::PacketTransportInternal* channel) = 0;
};

}  // namespace webrtc

#endif  // API_TRANSPORT_SCTP_TRANSPORT_FACTORY_INTERFACE_H_

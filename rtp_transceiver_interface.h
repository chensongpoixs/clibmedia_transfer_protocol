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


#ifndef _C_API_RTP_TRANSCEIVER_INTERFACE_H_
#define _C_API_RTP_TRANSCEIVER_INTERFACE_H_

#include <string>
#include <vector>

//#include "absl/base/attributes.h"
//#include "absl/types/optional.h"
//#include "api/array_view.h"
#include "libmedia_transfer_protocol/media_types.h"
//#include "libmedia_transfer_protocol/rtp_parameters.h"
//#include "libmedia_transfer_protocol/rtp_receiver_interface.h"
//#include "libmedia_transfer_protocol/rtp_sender_interface.h"
#include "libmedia_transfer_protocol/rtp_transceiver_direction.h"
//#include "api/scoped_refptr.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/system/rtc_export.h"

namespace libmedia_transfer_protocol {
	 
struct   RtpTransceiverInit   { 
  libmedia_transfer_protocol::RtpTransceiverDirection direction = libmedia_transfer_protocol::RtpTransceiverDirection::kSendRecv;

  // The added RtpTransceiver will be added to these streams.
  std::vector<std::string> stream_ids;

  // TODO(bugs.webrtc.org/7600): Not implemented.
 // std::vector<RtpEncodingParameters> send_encodings;
};

 
class   RtpTransceiverInterface : public rtc::RefCountInterface {
 public: 
  virtual libmedia_transfer_protocol::MediaType media_type() const = 0;

   
 protected:
  ~RtpTransceiverInterface() override = default;
};

}  // namespace webrtc

#endif  // API_RTP_TRANSCEIVER_INTERFACE_H_

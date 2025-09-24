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


#ifndef _C_MODULES_RTP_RTCP_SOURCE_FEC_PRIVATE_TABLES_RANDOM_H_
#define _C_MODULES_RTP_RTCP_SOURCE_FEC_PRIVATE_TABLES_RANDOM_H_

// This file contains a set of packets masks for the FEC code. The masks in
// this table are specifically designed to favor recovery to random loss.
// These packet masks are defined to protect up to maximum of 48 media packets.

#include <stdint.h>

namespace libmedia_transfer_protocol {
namespace fec_private_tables {

extern const uint8_t kPacketMaskRandomTbl[];

}  // namespace fec_private_tables
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_FEC_PRIVATE_TABLES_RANDOM_H_

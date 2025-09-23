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


#ifndef _C_MEDIA_BASE_TURN_UTILS_H_
#define _C_MEDIA_BASE_TURN_UTILS_H_

#include <cstddef>
#include <cstdint>

#include "rtc_base/system/rtc_export.h"

namespace libmedia_transfer_protocol {

// Finds data location within a TURN Channel Message or TURN Send Indication
// message.
bool RTC_EXPORT UnwrapTurnPacket(const uint8_t* packet,
                                 size_t packet_size,
                                 size_t* content_position,
                                 size_t* content_size);

}  // namespace cricket

#endif  // MEDIA_BASE_TURN_UTILS_H_

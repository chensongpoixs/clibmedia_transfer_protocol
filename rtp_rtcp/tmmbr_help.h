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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_TMMBR_HELP_H_
#define _C_MODULES_RTP_RTCP_SOURCE_TMMBR_HELP_H_

#include <stdint.h>

#include <vector>

#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/tmmb_item.h"

namespace libmedia_transfer_protocol {

class TMMBRHelp {
 public:
  static std::vector<rtcp::TmmbItem> FindBoundingSet(
      std::vector<rtcp::TmmbItem> candidates);

  static bool IsOwner(const std::vector<rtcp::TmmbItem>& bounding,
                      uint32_t ssrc);

  static uint64_t CalcMinBitrateBps(
      const std::vector<rtcp::TmmbItem>& candidates);
};
}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_TMMBR_HELP_H_

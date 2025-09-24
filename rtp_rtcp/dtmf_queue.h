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



#ifndef _C_MODULES_RTP_RTCP_SOURCE_DTMF_QUEUE_H_
#define _C_MODULES_RTP_RTCP_SOURCE_DTMF_QUEUE_H_

#include <stdint.h>

#include <list>

#include "rtc_base/synchronization/mutex.h"

namespace libmedia_transfer_protocol {
class DtmfQueue {
 public:
  struct Event {
    uint16_t duration_ms = 0;
    uint8_t payload_type = 0;
    uint8_t key = 0;
    uint8_t level = 0;
  };

  DtmfQueue();
  ~DtmfQueue();

  bool AddDtmf(const Event& event);
  bool NextDtmf(Event* event);
  bool PendingDtmf() const;

 private:
  mutable webrtc::Mutex dtmf_mutex_;
  std::list<Event> queue_;
};
}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_DTMF_QUEUE_H_

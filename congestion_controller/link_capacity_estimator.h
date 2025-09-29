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
				   date:  2025-09-30


				   Á´Â·ÈÝÁ¿

 ******************************************************************************/

#ifndef  _C_LINK_CAPACITY_ESTIMATOR_H_
#define  _C_LINK_CAPACITY_ESTIMATOR_H_

#include "absl/types/optional.h"
#include "api/units/data_rate.h"

namespace libmedia_transfer_protocol {
class LinkCapacityEstimator {
 public:
  LinkCapacityEstimator();
  webrtc::DataRate UpperBound() const;
  webrtc::DataRate LowerBound() const;
  void Reset();
  void OnOveruseDetected(webrtc::DataRate acknowledged_rate);
  void OnProbeRate(webrtc::DataRate probe_rate);
  bool has_estimate() const;
  webrtc::DataRate estimate() const;

 private:
  friend class GoogCcStatePrinter;
  void Update(webrtc::DataRate capacity_sample, double alpha);

  double deviation_estimate_kbps() const;
  absl::optional<double> estimate_kbps_;
  double deviation_kbps_ = 0.4;
};
}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_LINK_CAPACITY_ESTIMATOR_H_

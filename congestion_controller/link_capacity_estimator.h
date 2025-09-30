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


				   链路容量的大小计算

				   利用吞吐量和码率控制状态来初估链路的容量，作为码率调整的重要参考。
	 链路容量估计策略

	   取码率控制状态为 kDecrease（下调码率）时的吞吐量为估计样本，因为此时已经出现了过载情况，
	   当前的吞吐量可认为已经达到了链路容量瓶颈。 
	   
	   对吞吐量样本按照 95 分位进行指数平滑得到链路容量的估计值
	   
	   计算链路容量估计值的方差，并且进行归一化
	   
	   链路容量估计值加减 3 倍的标准差，得到链路容量的上限和下限。按照统计学原理，正负 3 倍的标准差范围，可以覆盖 99.6%的置信区间。
 ******************************************************************************/

#ifndef  _C_LINK_CAPACITY_ESTIMATOR_H_
#define  _C_LINK_CAPACITY_ESTIMATOR_H_

#include "absl/types/optional.h"
#include "api/units/data_rate.h"

namespace libmedia_transfer_protocol {
class LinkCapacityEstimator {
 public:
  LinkCapacityEstimator();
   // 返回估计值 上下限
  webrtc::DataRate UpperBound() const;
  webrtc::DataRate LowerBound() const;
  void Reset();

   // 探测网络出现包   样本数据输入 估计容量的值
  void OnOveruseDetected(webrtc::DataRate acknowledged_rate);
  void OnProbeRate(webrtc::DataRate probe_rate);
  bool has_estimate() const;
  webrtc::DataRate estimate() const;

 private:
  friend class GoogCcStatePrinter;
  void Update(webrtc::DataRate capacity_sample, double alpha);

  // 估计值误差值
  double deviation_estimate_kbps() const;

  // 估计出来链路的容量
  absl::optional<double> estimate_kbps_;
  // 归一化指数的 的方差
  double deviation_kbps_ = 0.4;
};
}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_LINK_CAPACITY_ESTIMATOR_H_

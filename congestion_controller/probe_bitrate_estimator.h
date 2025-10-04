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
				   date:  2025-10-04

				   探测比特率估算器

				   发送速率  = 发送数据大小 / 发送时间
				   确认接受速率 = 接受数据的大小 / 接受时间



				   优化： 发送码流 * 系数 <= 网络链路已经负载或者饱和了 探测码流降低一定系数


 ******************************************************************************/


#ifndef _C__PROBE_BITRATE_ESTIMATOR_H_
#define _C__PROBE_BITRATE_ESTIMATOR_H_

#include <limits>
#include <map>

#include "absl/types/optional.h"
#include "libice/network_types.h"
#include "api/units/data_rate.h"

namespace libmedia_transfer_protocol {
//class RtcEventLog;

class ProbeBitrateEstimator {
 public:
  explicit ProbeBitrateEstimator(/*RtcEventLog* event_log*/);
  ~ProbeBitrateEstimator();

  // Should be called for every probe packet we receive feedback about.
  // Returns the estimated bitrate if the probe completes a valid cluster.
  absl::optional<webrtc::DataRate> HandleProbeAndEstimateBitrate(
      const libice::PacketResult& packet_feedback);

  // 取完结果 就释放了结果了  只能使用一次结果   探测码流结果
  absl::optional<webrtc::DataRate> FetchAndResetLastEstimatedBitrate();

 private:
  struct AggregatedCluster {
	  //接受端确认山东的探测包的总个数
    int num_probes = 0;
	//第一次发送探测数据包的时间
    webrtc::Timestamp first_send = webrtc::Timestamp::PlusInfinity();
	// 最后一次发送探测包的时间
    webrtc::Timestamp last_send = webrtc::Timestamp::MinusInfinity();
	//第一次接受探测数据包的时间
    webrtc::Timestamp first_receive = webrtc::Timestamp::PlusInfinity();
	// 最后一次探测数据包的时间
    webrtc::Timestamp last_receive = webrtc::Timestamp::MinusInfinity();
	// 最后一次发送探测数据包的大小
    webrtc::DataSize size_last_send = webrtc::DataSize::Zero();
	// 第一次接受端接受到数据包的大小
    webrtc::DataSize size_first_receive = webrtc::DataSize::Zero();
	// 统计接收端确认接受的探测包的总字节数
    webrtc::DataSize size_total = webrtc::DataSize::Zero();
  };

  // Erases old cluster data that was seen before `timestamp`.
  void EraseOldClusters(webrtc::Timestamp timestamp);

  std::map<int, AggregatedCluster> clusters_;
 /// RtcEventLog* const event_log_;

  // 估计出来的结果码流
  absl::optional<webrtc::DataRate> estimated_data_rate_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_PROBE_BITRATE_ESTIMATOR_H_

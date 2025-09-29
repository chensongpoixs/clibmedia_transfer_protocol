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
				   date:  2025-09-29


				   延迟趋势

				   线性回归 最小二乘法 ===> 直线 y = ax + b 


				   线性回归函数最小二乘法 trend

 1. trend值含义
	  ①   表示延迟趋势，也是线性回归最小二乘法拟合的直线斜率
	  ② 当发送码率 send_rate 小于网络链路的容量 link_capacity
		时，数据包不会在网络队列中排队积压，延迟趋势 trend 值基本为
		0（网络存在波动，不可能完全等于0）
	  ③ 当发送码率 send_rate 大于网络链路的容量 link_capacity
		时，数据包会在网络队列中排队积压，数据包的传输延迟逐步增大，此时的延迟趋势 trend
		值 > 0，表示网络出现拥塞
	  ④ 当网络拥塞得到缓解时，网络队列会逐渐排空，数据包的传输延迟开始下降，此时的延迟趋势trend 值 < 0
	  ⑤ trend 等价于 estimate ((send_rate – link_capacity) / link_capacity)


2. trend

	值我们可以利用包组延迟样本数据，通过线性回归最小二乘法计算斜率获得，有了 trend
	值我们就能判断网络是否出现过载拥塞，然后调整发送码率
	send_rate，使得网络不处于过载或者负载过低的状态，此时的发送码率就接近于网络的容量，即比较准确的网络带宽估计值。
	 
	 
	


	三种网络状态: 1. 正常状态 2. 低负载状态 3. 过载状态
 ******************************************************************************/
#ifndef _C_TRENDLINE_ESTIMATOR_H_
#define _C_TRENDLINE_ESTIMATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <memory>
#include <utility>

#include "libmedia_transfer_protocol/network_state_predictor.h"
//#include "api/transport/webrtc_key_value_config.h"
#include "libmedia_transfer_protocol/congestion_controller/delay_increase_detector_interface.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/experiments/struct_parameters_parser.h"

namespace libmedia_transfer_protocol {

struct TrendlineEstimatorSettings {
  static constexpr char kKey[] = "WebRTC-Bwe-TrendlineEstimatorSettings";
  static constexpr unsigned kDefaultTrendlineWindowSize = 20;
  TrendlineEstimatorSettings();
  //TrendlineEstimatorSettings() = delete;
 // explicit TrendlineEstimatorSettings(
   //   /*const WebRtcKeyValueConfig* key_value_config*/);

  // Sort the packets in the window. Should be redundant,
  // but then almost no cost.
  bool enable_sort = false;

  // Cap the trendline slope based on the minimum delay seen
  // in the beginning_packets and end_packets respectively.
  bool enable_cap = false;
  unsigned beginning_packets = 7;
  unsigned end_packets = 7;
  double cap_uncertainty = 0.0;

  // Size (in packets) of the window.
  unsigned window_size = kDefaultTrendlineWindowSize;

  std::unique_ptr<webrtc::StructParametersParser> Parser();
};

class TrendlineEstimator : public DelayIncreaseDetectorInterface {
 public:
  TrendlineEstimator(/*const WebRtcKeyValueConfig* key_value_config,*/
                     NetworkStatePredictor* network_state_predictor);

  ~TrendlineEstimator() override;

  // Update the estimator with a new sample. The deltas should represent deltas
  // between timestamp groups as defined by the InterArrival class.
  // 
  void Update(double recv_delta_ms/*接受延迟差*/,
              double send_delta_ms/*发送延迟差*/,
              int64_t send_time_ms/*发送时间*/,
              int64_t arrival_time_ms/**/,
              size_t packet_size/*数据*/,
              bool calculated_deltas/*是否有效*/) override;

  void UpdateTrendline(double recv_delta_ms,
                       double send_delta_ms,
                       int64_t send_time_ms,
                       int64_t arrival_time_ms,
                       size_t packet_size);

  BandwidthUsage State() const override;

  struct PacketTiming {
    PacketTiming(double arrival_time_ms,
                 double smoothed_delay_ms,
                 double raw_delay_ms)
        : arrival_time_ms(arrival_time_ms),
          smoothed_delay_ms(smoothed_delay_ms),
          raw_delay_ms(raw_delay_ms) {}
	//包到达时间
    double arrival_time_ms;
	// 指数平滑之后延迟差
    double smoothed_delay_ms;
	// 原始延迟差
    double raw_delay_ms;
  };

 private:
  friend class GoogCcStatePrinter;
  void Detect(double trend, double ts_delta, int64_t now_ms);
  // 阈值自适应调整
  void UpdateThreshold(double modified_offset, int64_t now_ms);

  // Parameters.
  TrendlineEstimatorSettings settings_;
  //平滑系数    历史数据的平滑
  const double smoothing_coef_;
  //  增益的阈值
  const double threshold_gain_;
  // Used by the existing threshold.
  // 统计样本的个数
  int num_of_deltas_;
  // Keep the arrival times small by using the change from the first packet.
  int64_t first_arrival_time_ms_;
  // Exponential backoff filtering.
   // 统计延迟
  double accumulated_delay_;
  double smoothed_delay_;
  // Linear least squares regression.
  std::deque<PacketTiming> delay_hist_;

  const double k_up_;
  const double k_down_;
  double overusing_time_threshold_;

  // 后续会动态自适应调整
  double threshold_;
  double prev_modified_trend_;
  // 阈值保存的时间
  int64_t last_update_ms_;
  double prev_trend_;
  // 过载统计的时间
  double time_over_using_;
  // 过载
  int overuse_counter_;
  // 网络状态
  BandwidthUsage hypothesis_;
  BandwidthUsage hypothesis_predicted_;
  NetworkStatePredictor* network_state_predictor_;

 // RTC_DISALLOW_COPY_AND_ASSIGN(TrendlineEstimator);
};
}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_

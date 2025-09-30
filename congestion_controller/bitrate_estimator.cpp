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

				   贝叶斯估计出网络 吞吐量


 ******************************************************************************/

#include "libmedia_transfer_protocol/congestion_controller/bitrate_estimator.h"

#include <stdio.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "api/units/data_rate.h"
//#include "modules/remote_bitrate_estimator/test/bwe_test_logging.h"
#include "rtc_base/logging.h"

namespace libmedia_transfer_protocol {

namespace {
	//初始化窗口 500ms 
constexpr int kInitialRateWindowMs = 500;
    // 正常的窗口150ms
constexpr int kRateWindowMs = 150;
constexpr int kMinRateWindowMs = 150;
constexpr int kMaxRateWindowMs = 1000;

const char kBweThroughputWindowConfig[] = "WebRTC-BweThroughputWindowConfig";

}  // namespace

BitrateEstimator::BitrateEstimator(/*const WebRtcKeyValueConfig* key_value_config*/)
    : sum_(0),
      initial_window_ms_("initial_window_ms",
                         kInitialRateWindowMs,
                         kMinRateWindowMs,
                         kMaxRateWindowMs),
      noninitial_window_ms_("window_ms",
                            kRateWindowMs,
                            kMinRateWindowMs,
                            kMaxRateWindowMs),
      uncertainty_scale_("scale", 10.0),
      uncertainty_scale_in_alr_("scale_alr", uncertainty_scale_),
      small_sample_uncertainty_scale_("scale_small", uncertainty_scale_),
      small_sample_threshold_("small_thresh", webrtc::DataSize::Zero()),
      uncertainty_symmetry_cap_("symmetry_cap", webrtc::DataRate::Zero()),
      estimate_floor_("floor", webrtc::DataRate::Zero()),
      current_window_ms_(0),
      prev_time_ms_(-1),
      bitrate_estimate_kbps_(-1.0f),
      bitrate_estimate_var_(50.0f) {
  // E.g WebRTC-BweThroughputWindowConfig/initial_window_ms:350,window_ms:250/
  //ParseFieldTrial(
  //    {&initial_window_ms_, &noninitial_window_ms_, &uncertainty_scale_,
  //     &uncertainty_scale_in_alr_, &small_sample_uncertainty_scale_,
  //     &small_sample_threshold_, &uncertainty_symmetry_cap_, &estimate_floor_},
  //    key_value_config->Lookup(kBweThroughputWindowConfig));
}

BitrateEstimator::~BitrateEstimator() = default;
/**
* 1. 观测码率
* 2. 预估码率
*/
void BitrateEstimator::Update(webrtc::Timestamp at_time, webrtc::DataSize amount, bool in_alr) {
  int rate_window_ms = noninitial_window_ms_.Get();
  // We use a larger window at the beginning to get a more stable sample that
  // we can use to initialize the estimate.
  if (bitrate_estimate_kbps_ < 0.f)
  {
	  //刚刚启动时使用较大窗口 500ms使用数据比较好
	  rate_window_ms = initial_window_ms_.Get();
  }
  bool is_small_sample = false;
   // 计算窗口内平均码率公式： bitrate_sample=  (窗口内总字节数×8)/窗口时间跨度（秒）
  float bitrate_sample_kbps = UpdateWindow(at_time.ms(), amount.bytes(),
                                           rate_window_ms, &is_small_sample);
  if (bitrate_sample_kbps < 0.0f)
  {
	  return;
  }
  // 先验码率值
  if (bitrate_estimate_kbps_ < 0.0f) {
	  // 第一次获取样本数据
    // This is the very first sample we get. Use it to initialize the estimate.
    bitrate_estimate_kbps_ = bitrate_sample_kbps;
    return;
  }
  // Optionally use higher uncertainty for very small samples to avoid dropping
  // estimate and for samples obtained in ALR.
  // TODO: 根据样本的不同， 可以给不同的不确定性， 比如小样本
  //  给于更大的不确定性
  float scale = uncertainty_scale_ /*10.0f*/;
  if (is_small_sample && bitrate_sample_kbps < bitrate_estimate_kbps_)
  {
    scale = small_sample_uncertainty_scale_;
  }
  else if (in_alr && bitrate_sample_kbps < bitrate_estimate_kbps_) 
  {
    // Optionally use higher uncertainty for samples obtained during ALR.
    scale = uncertainty_scale_in_alr_;
  }
  // Define the sample uncertainty as a function of how far away it is from the
  // current estimate. With low values of uncertainty_symmetry_cap_ we add more
  // uncertainty to increases than to decreases. For higher values we approach
  // symmetry.
  //  计算后验分布方差 = 样本方差
  float sample_uncertainty =
      scale * std::abs(bitrate_estimate_kbps_ - bitrate_sample_kbps) /
      (bitrate_estimate_kbps_ +
       std::min(bitrate_sample_kbps,
                uncertainty_symmetry_cap_.Get().kbps<float>()));

  float sample_var = sample_uncertainty * sample_uncertainty;
  // Update a bayesian estimate of the rate, weighting it lower if the sample
  // uncertainty is large.
  // The bitrate estimate uncertainty is increased with each update to model
  // that the bitrate changes over time.
  //  误差5.0f估计值的不确定性 需要增加
  float pred_bitrate_estimate_var = bitrate_estimate_var_ + 5.f;
  bitrate_estimate_kbps_ = (sample_var * bitrate_estimate_kbps_ +
                            pred_bitrate_estimate_var * bitrate_sample_kbps) /
                           (sample_var + pred_bitrate_estimate_var);
  bitrate_estimate_kbps_ =
      std::max(bitrate_estimate_kbps_, estimate_floor_.Get().kbps<float>());
  bitrate_estimate_var_ = sample_var * pred_bitrate_estimate_var /
                          (sample_var + pred_bitrate_estimate_var);
 // BWE_TEST_LOGGING_PLOT(1, "acknowledged_bitrate", at_time.ms(),
  //                      bitrate_estimate_kbps_ * 1000);

 // RTC_LOG(LS_INFO) << "ack_b"
}

float BitrateEstimator::UpdateWindow(int64_t now_ms,
                                     int bytes,
                                     int rate_window_ms,
                                     bool* is_small_sample) 
{
  RTC_DCHECK(is_small_sample != nullptr);
  // Reset if time moves backwards.
  // 如果时间发生向前移动 需要重置数据
  if (now_ms < prev_time_ms_) {
    prev_time_ms_ = -1;
    sum_ = 0;
    current_window_ms_ = 0;
  }
  if (prev_time_ms_ >= 0) {
    current_window_ms_ += now_ms - prev_time_ms_;
    // Reset if nothing has been received for more than a full window.
	// 如果超过一个或者多个完整的时间窗口， 没有收到任何数据， 需要重置数据
    if (now_ms - prev_time_ms_ > rate_window_ms) {
      sum_ = 0;
      current_window_ms_ %= rate_window_ms;// 
    }
  }
  prev_time_ms_ = now_ms;
  float bitrate_sample = -1.0f;
  if (current_window_ms_ >= rate_window_ms) {
    *is_small_sample = sum_ < small_sample_threshold_->bytes();
	//  码流的计算
    bitrate_sample = 8.0f * sum_ / static_cast<float>(rate_window_ms);
    current_window_ms_ -= rate_window_ms;
    sum_ = 0;
  }
  sum_ += bytes;
  return bitrate_sample;
}

absl::optional<webrtc::DataRate> BitrateEstimator::bitrate() const {
  if (bitrate_estimate_kbps_ < 0.f)
    return absl::nullopt;
  return webrtc::DataRate::KilobitsPerSec(bitrate_estimate_kbps_);
}

absl::optional<webrtc::DataRate> BitrateEstimator::PeekRate() const {
  if (current_window_ms_ > 0)
    return webrtc::DataSize::Bytes(sum_) /webrtc:: TimeDelta::Millis(current_window_ms_);
  return absl::nullopt;
}

void BitrateEstimator::ExpectFastRateChange() {
  // By setting the bitrate-estimate variance to a higher value we allow the
  // bitrate to change fast for the next few samples.
  bitrate_estimate_var_ += 200;
}

}  // namespace webrtc

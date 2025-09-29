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


				   �ӳ�����

				   ���Իع� ��С���˷� ===> ֱ�� y = ax + b 


				   ���Իع麯����С���˷� trend

 1. trendֵ����
	  ��   ��ʾ�ӳ����ƣ�Ҳ�����Իع���С���˷���ϵ�ֱ��б��
	  �� ���������� send_rate С��������·������ link_capacity
		ʱ�����ݰ�����������������Ŷӻ�ѹ���ӳ����� trend ֵ����Ϊ
		0��������ڲ�������������ȫ����0��
	  �� ���������� send_rate ����������·������ link_capacity
		ʱ�����ݰ���������������Ŷӻ�ѹ�����ݰ��Ĵ����ӳ������󣬴�ʱ���ӳ����� trend
		ֵ > 0����ʾ�������ӵ��
	  �� ������ӵ���õ�����ʱ��������л����ſգ����ݰ��Ĵ����ӳٿ�ʼ�½�����ʱ���ӳ�����trend ֵ < 0
	  �� trend �ȼ��� estimate ((send_rate �C link_capacity) / link_capacity)


2. trend

	ֵ���ǿ������ð����ӳ��������ݣ�ͨ�����Իع���С���˷�����б�ʻ�ã����� trend
	ֵ���Ǿ����ж������Ƿ���ֹ���ӵ����Ȼ�������������
	send_rate��ʹ�����粻���ڹ��ػ��߸��ع��͵�״̬����ʱ�ķ������ʾͽӽ�����������������Ƚ�׼ȷ������������ֵ��
	 
	 
	


	��������״̬: 1. ����״̬ 2. �͸���״̬ 3. ����״̬
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
  void Update(double recv_delta_ms/*�����ӳٲ�*/,
              double send_delta_ms/*�����ӳٲ�*/,
              int64_t send_time_ms/*����ʱ��*/,
              int64_t arrival_time_ms/**/,
              size_t packet_size/*����*/,
              bool calculated_deltas/*�Ƿ���Ч*/) override;

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
	//������ʱ��
    double arrival_time_ms;
	// ָ��ƽ��֮���ӳٲ�
    double smoothed_delay_ms;
	// ԭʼ�ӳٲ�
    double raw_delay_ms;
  };

 private:
  friend class GoogCcStatePrinter;
  void Detect(double trend, double ts_delta, int64_t now_ms);
  // ��ֵ����Ӧ����
  void UpdateThreshold(double modified_offset, int64_t now_ms);

  // Parameters.
  TrendlineEstimatorSettings settings_;
  //ƽ��ϵ��    ��ʷ���ݵ�ƽ��
  const double smoothing_coef_;
  //  �������ֵ
  const double threshold_gain_;
  // Used by the existing threshold.
  // ͳ�������ĸ���
  int num_of_deltas_;
  // Keep the arrival times small by using the change from the first packet.
  int64_t first_arrival_time_ms_;
  // Exponential backoff filtering.
   // ͳ���ӳ�
  double accumulated_delay_;
  double smoothed_delay_;
  // Linear least squares regression.
  std::deque<PacketTiming> delay_hist_;

  const double k_up_;
  const double k_down_;
  double overusing_time_threshold_;

  // �����ᶯ̬����Ӧ����
  double threshold_;
  double prev_modified_trend_;
  // ��ֵ�����ʱ��
  int64_t last_update_ms_;
  double prev_trend_;
  // ����ͳ�Ƶ�ʱ��
  double time_over_using_;
  // ����
  int overuse_counter_;
  // ����״̬
  BandwidthUsage hypothesis_;
  BandwidthUsage hypothesis_predicted_;
  NetworkStatePredictor* network_state_predictor_;

 // RTC_DISALLOW_COPY_AND_ASSIGN(TrendlineEstimator);
};
}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_TRENDLINE_ESTIMATOR_H_

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

				   ̽������ʹ�����

				   ��������  = �������ݴ�С / ����ʱ��
				   ȷ�Ͻ������� = �������ݵĴ�С / ����ʱ��



				   �Ż��� �������� * ϵ�� <= ������·�Ѿ����ػ��߱����� ̽����������һ��ϵ��


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

  // ȡ���� ���ͷ��˽����  ֻ��ʹ��һ�ν��   ̽���������
  absl::optional<webrtc::DataRate> FetchAndResetLastEstimatedBitrate();

 private:
  struct AggregatedCluster {
	  //���ܶ�ȷ��ɽ����̽������ܸ���
    int num_probes = 0;
	//��һ�η���̽�����ݰ���ʱ��
    webrtc::Timestamp first_send = webrtc::Timestamp::PlusInfinity();
	// ���һ�η���̽�����ʱ��
    webrtc::Timestamp last_send = webrtc::Timestamp::MinusInfinity();
	//��һ�ν���̽�����ݰ���ʱ��
    webrtc::Timestamp first_receive = webrtc::Timestamp::PlusInfinity();
	// ���һ��̽�����ݰ���ʱ��
    webrtc::Timestamp last_receive = webrtc::Timestamp::MinusInfinity();
	// ���һ�η���̽�����ݰ��Ĵ�С
    webrtc::DataSize size_last_send = webrtc::DataSize::Zero();
	// ��һ�ν��ܶ˽��ܵ����ݰ��Ĵ�С
    webrtc::DataSize size_first_receive = webrtc::DataSize::Zero();
	// ͳ�ƽ��ն�ȷ�Ͻ��ܵ�̽��������ֽ���
    webrtc::DataSize size_total = webrtc::DataSize::Zero();
  };

  // Erases old cluster data that was seen before `timestamp`.
  void EraseOldClusters(webrtc::Timestamp timestamp);

  std::map<int, AggregatedCluster> clusters_;
 /// RtcEventLog* const event_log_;

  // ���Ƴ����Ľ������
  absl::optional<webrtc::DataRate> estimated_data_rate_;
};

}  // namespace webrtc

#endif  // MODULES_CONGESTION_CONTROLLER_GOOG_CC_PROBE_BITRATE_ESTIMATOR_H_

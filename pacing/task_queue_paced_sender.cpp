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
				   date:  2025-09-28



 ******************************************************************************/


#include "libmedia_transfer_protocol/pacing/task_queue_paced_sender.h"

#include <algorithm>
#include <utility>
#include "absl/memory/memory.h"
#include "rtc_base/checks.h"
#include "rtc_base/event.h"
#include "rtc_base/logging.h"
#include "rtc_base/task_utils/to_queued_task.h"
#include "rtc_base/trace_event.h"

namespace libmedia_transfer_protocol {
namespace {
// If no calls to MaybeProcessPackets() happen, make sure we update stats
// at least every `kMaxTimeBetweenStatsUpdates` as long as the pacer isn't
// completely drained.
constexpr webrtc::TimeDelta kMaxTimeBetweenStatsUpdates = webrtc::TimeDelta::Millis(33);
// Don't call UpdateStats() more than `kMinTimeBetweenStatsUpdates` apart,
// for performance reasons.
constexpr webrtc::TimeDelta kMinTimeBetweenStatsUpdates = webrtc::TimeDelta::Millis(1);
}  // namespace

TaskQueuePacedSender::TaskQueuePacedSender(
	webrtc::Clock* clock,
    PacingController::PacketSender* packet_sender,
	//webrtc::RtcEventLog* event_log,
  //  const webrtc::WebRtcKeyValueConfig* field_trials,
	webrtc::TaskQueueFactory* task_queue_factory,
	webrtc::TimeDelta hold_back_window)
    : clock_(clock),
      hold_back_window_(hold_back_window),
      pacing_controller_(clock,
                         packet_sender,
                        // event_log,
                        // field_trials,
                         PacingController::ProcessMode::kDynamic),
      next_process_time_(webrtc::Timestamp::MinusInfinity()),
      stats_update_scheduled_(false),
      last_stats_time_(webrtc::Timestamp::MinusInfinity()),
      is_shutdown_(false),
      task_queue_(task_queue_factory->CreateTaskQueue(
          "TaskQueuePacedSender",
		  webrtc::TaskQueueFactory::Priority::NORMAL)) 
{

	// ����������ʼ�� pacing ���ʹ�С
	pacing_controller_.SetPacingRates(webrtc::DataRate::KilobitsPerSec(1500)
	, webrtc::DataRate::KilobitsPerSec(1500));
}

TaskQueuePacedSender::~TaskQueuePacedSender() {
  // Post an immediate task to mark the queue as shutting down.
  // The rtc::TaskQueue destructor will wait for pending tasks to
  // complete before continuing.
  task_queue_.PostTask([&]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    is_shutdown_ = true;
  });
}

void TaskQueuePacedSender::EnsureStarted() {
  task_queue_.PostTask([this]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    is_started_ = true;
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::CreateProbeCluster(webrtc::DataRate bitrate,
                                              int cluster_id) {
  task_queue_.PostTask([this, bitrate, cluster_id]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.CreateProbeCluster(bitrate, cluster_id);
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::Pause() {
  task_queue_.PostTask([this]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.Pause();
  });
}

void TaskQueuePacedSender::Resume() {
  task_queue_.PostTask([this]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.Resume();
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::SetCongestionWindow(
	webrtc::DataSize congestion_window_size) {
  task_queue_.PostTask([this, congestion_window_size]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetCongestionWindow(congestion_window_size);
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::UpdateOutstandingData(webrtc::DataSize outstanding_data) {
  if (task_queue_.IsCurrent()) {
    RTC_DCHECK_RUN_ON(&task_queue_);
    // Fast path since this can be called once per sent packet while on the
    // task queue.
    pacing_controller_.UpdateOutstandingData(outstanding_data);
    return;
  }

  task_queue_.PostTask([this, outstanding_data]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.UpdateOutstandingData(outstanding_data);
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::SetPacingRates(webrtc::DataRate pacing_rate,
	webrtc::DataRate padding_rate) {
  task_queue_.PostTask([this, pacing_rate, padding_rate]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetPacingRates(pacing_rate, padding_rate);
	// ��������еİ�
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::EnqueuePackets(
    std::vector<std::unique_ptr<RtpPacketToSend>> packets) {
#if RTC_TRACE_EVENTS_ENABLED
  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("webrtc"),
               "TaskQueuePacedSender::EnqueuePackets");
  for (auto& packet : packets) {
    TRACE_EVENT2(TRACE_DISABLED_BY_DEFAULT("webrtc"),
                 "TaskQueuePacedSender::EnqueuePackets::Loop",
                 "sequence_number", packet->SequenceNumber(), "rtp_timestamp",
                 packet->Timestamp());
  }
#endif

  task_queue_.PostTask([this, packets_ = std::move(packets)]() mutable {
    RTC_DCHECK_RUN_ON(&task_queue_);
    for (auto& packet : packets_) {
      RTC_DCHECK_GE(packet->capture_time_ms(), 0);
      pacing_controller_.EnqueuePacket(std::move(packet));
    }
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

void TaskQueuePacedSender::SetAccountForAudioPackets(bool account_for_audio) {
  task_queue_.PostTask([this, account_for_audio]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetAccountForAudioPackets(account_for_audio);
  });
}

void TaskQueuePacedSender::SetIncludeOverhead() {
  task_queue_.PostTask([this]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetIncludeOverhead();
  });
}

void TaskQueuePacedSender::SetTransportOverhead(webrtc::DataSize overhead_per_packet) {
  task_queue_.PostTask([this, overhead_per_packet]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetTransportOverhead(overhead_per_packet);
  });
}

void TaskQueuePacedSender::SetQueueTimeLimit(webrtc::TimeDelta limit) {
  task_queue_.PostTask([this, limit]() {
    RTC_DCHECK_RUN_ON(&task_queue_);
    pacing_controller_.SetQueueTimeLimit(limit);
    MaybeProcessPackets(webrtc::Timestamp::MinusInfinity());
  });
}

webrtc::TimeDelta TaskQueuePacedSender::ExpectedQueueTime() const {
  return GetStats().expected_queue_time;
}

webrtc::DataSize TaskQueuePacedSender::QueueSizeData() const {
  return GetStats().queue_size;
}

absl::optional<webrtc::Timestamp> TaskQueuePacedSender::FirstSentPacketTime() const {
  return GetStats().first_sent_packet_time;
}

webrtc::TimeDelta TaskQueuePacedSender::OldestPacketWaitTime() const {
  return GetStats().oldest_packet_wait_time;
}

void TaskQueuePacedSender::OnStatsUpdated(const Stats& stats) {
	webrtc::MutexLock lock(&stats_mutex_);
  current_stats_ = stats;
}

void TaskQueuePacedSender::MaybeProcessPackets(
	webrtc::Timestamp scheduled_process_time) {
  RTC_DCHECK_RUN_ON(&task_queue_);

  if (is_shutdown_ || !is_started_) {
    return;
  }

  // Normally, run ProcessPackets() only if this is the scheduled task.
  // If it is not but it is already time to process and there either is
  // no scheduled task or the schedule has shifted forward in time, run
  // anyway and clear any schedule.
  webrtc::Timestamp next_process_time = pacing_controller_.NextSendTime();
  const webrtc::Timestamp now = clock_->CurrentTime();
  const bool is_scheduled_call = next_process_time_ == scheduled_process_time;
  if (is_scheduled_call) {
    // Indicate no pending scheduled call.
	   // ��ǰ�����񽫱�ִ�У���Ҫ�����趨��һ������ִ�е�ʱ��
    next_process_time_ = webrtc::Timestamp::MinusInfinity();
  }
  if (is_scheduled_call ||
      (now >= next_process_time && (next_process_time_.IsInfinite() ||
                                    next_process_time < next_process_time_))) {
	  // ִ�����ݰ������߼�
    pacing_controller_.ProcessPackets();
    next_process_time = pacing_controller_.NextSendTime();
  }
  // ��Ҫ���೤ʱ��֮���ٴν����������
  absl::optional<webrtc::TimeDelta> time_to_next_process;
  if (pacing_controller_.IsProbing() &&
      next_process_time != next_process_time_) {
    // If we're probing and there isn't already a wakeup scheduled for the next
    // process time, always post a task and just round sleep time down to
    // nearest millisecond.
    if (next_process_time.IsMinusInfinity()) {
      time_to_next_process = webrtc::TimeDelta::Zero();
    } else {
      time_to_next_process =
          std::max(webrtc::TimeDelta::Zero(),
                   (next_process_time - now).RoundDownTo(webrtc::TimeDelta::Millis(1)));
    }
  } else if (next_process_time_.IsMinusInfinity() ||
             next_process_time <= next_process_time_ - hold_back_window_) {
    // Schedule a new task since there is none currently scheduled
    // (`next_process_time_` is infinite), or the new process time is at least
    // one holdback window earlier than whatever is currently scheduled.
	  // ��ǰ��û���趨��һ�εĵ���������Ҫ����һ��
    time_to_next_process = std::max(next_process_time - now, hold_back_window_);
  }

  if (time_to_next_process) {
    // Set a new scheduled process time and post a delayed task.
    next_process_time_ = next_process_time;

    task_queue_.PostDelayedTask(
        [this, next_process_time]() { MaybeProcessPackets(next_process_time); },
        time_to_next_process->ms<uint32_t>());
  }

  MaybeUpdateStats(false);
}

void TaskQueuePacedSender::MaybeUpdateStats(bool is_scheduled_call) {
  if (is_shutdown_) {
    if (is_scheduled_call) {
      stats_update_scheduled_ = false;
    }
    return;
  }

  webrtc::Timestamp now = clock_->CurrentTime();
  if (is_scheduled_call) {
    // Allow scheduled task to process packets to clear up an remaining debt
    // level in an otherwise empty queue.
    pacing_controller_.ProcessPackets();
  } else {
    if (now - last_stats_time_ < kMinTimeBetweenStatsUpdates) {
      // Too frequent unscheduled stats update, return early.
      return;
    }
  }

  Stats new_stats;
  new_stats.expected_queue_time = pacing_controller_.ExpectedQueueTime();
  new_stats.first_sent_packet_time = pacing_controller_.FirstSentPacketTime();
  new_stats.oldest_packet_wait_time = pacing_controller_.OldestPacketWaitTime();
  new_stats.queue_size = pacing_controller_.QueueSizeData();
  OnStatsUpdated(new_stats);

  last_stats_time_ = now;

  bool pacer_drained = pacing_controller_.QueueSizePackets() == 0 &&
                       pacing_controller_.CurrentBufferLevel().IsZero();

  // If there's anything interesting to get from the pacer and this is a
  // scheduled call (or no scheduled call in flight), post a new scheduled stats
  // update.
  if (!pacer_drained) {
    if (!stats_update_scheduled_) {
      // There is no pending delayed task to update stats, add one.
      // Treat this call as being scheduled in order to bootstrap scheduling
      // loop.
      stats_update_scheduled_ = true;
      is_scheduled_call = true;
    }

    // Only if on the scheduled call loop do we want to schedule a new delayed
    // task.
    if (is_scheduled_call) {
      task_queue_.PostDelayedTask(
          [this]() {
            RTC_DCHECK_RUN_ON(&task_queue_);
            MaybeUpdateStats(true);
          },
          kMaxTimeBetweenStatsUpdates.ms<uint32_t>());
    }
  } else if (is_scheduled_call) {
    // This is a scheduled call, signing out since there's nothing interesting
    // left to check.
    stats_update_scheduled_ = false;
  }
}

TaskQueuePacedSender::Stats TaskQueuePacedSender::GetStats() const {
	webrtc::MutexLock lock(&stats_mutex_);
  return current_stats_;
}

}  // namespace webrtc

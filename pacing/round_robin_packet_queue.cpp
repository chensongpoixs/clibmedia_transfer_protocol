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


#include "libmedia_transfer_protocol/pacing/round_robin_packet_queue.h"

#include <algorithm>
#include <cstdint>
#include <utility>

#include "absl/strings/match.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {
namespace {
	// 两个视频流之间累计发送字节数的最大差值
static constexpr webrtc::DataSize kMaxLeadingSize = webrtc::DataSize::Bytes(1400);
}

RoundRobinPacketQueue::QueuedPacket::QueuedPacket(const QueuedPacket& rhs) =
    default;
RoundRobinPacketQueue::QueuedPacket::~QueuedPacket() = default;

RoundRobinPacketQueue::QueuedPacket::QueuedPacket(
    int priority,
	webrtc::Timestamp enqueue_time,
    uint64_t enqueue_order,
    std::multiset<webrtc::Timestamp>::iterator enqueue_time_it,
    std::unique_ptr<RtpPacketToSend> packet)
    : priority_(priority),
      enqueue_time_(enqueue_time),
      enqueue_order_(enqueue_order),
      is_retransmission_(packet->packet_type() ==
                         RtpPacketMediaType::kRetransmission),
      enqueue_time_it_(enqueue_time_it),
      owned_packet_(packet.release()) {}

bool RoundRobinPacketQueue::QueuedPacket::operator<(
    const RoundRobinPacketQueue::QueuedPacket& other) const {
  if (priority_ != other.priority_)
    return priority_ > other.priority_;
  if (is_retransmission_ != other.is_retransmission_)
    return other.is_retransmission_;

  return enqueue_order_ > other.enqueue_order_;
}

int RoundRobinPacketQueue::QueuedPacket::Priority() const {
  return priority_;
}

RtpPacketMediaType RoundRobinPacketQueue::QueuedPacket::Type() const {
  return *owned_packet_->packet_type();
}

uint32_t RoundRobinPacketQueue::QueuedPacket::Ssrc() const {
  return owned_packet_->Ssrc();
}

webrtc::Timestamp RoundRobinPacketQueue::QueuedPacket::EnqueueTime() const {
  return enqueue_time_;
}

bool RoundRobinPacketQueue::QueuedPacket::IsRetransmission() const {
  return Type() == RtpPacketMediaType::kRetransmission;
}

uint64_t RoundRobinPacketQueue::QueuedPacket::EnqueueOrder() const {
  return enqueue_order_;
}

RtpPacketToSend* RoundRobinPacketQueue::QueuedPacket::RtpPacket() const {
  return owned_packet_;
}

void RoundRobinPacketQueue::QueuedPacket::UpdateEnqueueTimeIterator(
    std::multiset<webrtc::Timestamp>::iterator it) {
  enqueue_time_it_ = it;
}

std::multiset<webrtc::Timestamp>::iterator
RoundRobinPacketQueue::QueuedPacket::EnqueueTimeIterator() const {
  return enqueue_time_it_;
}

void RoundRobinPacketQueue::QueuedPacket::SubtractPauseTime(
	webrtc::TimeDelta pause_time_sum) {
  enqueue_time_ -= pause_time_sum;
}

RoundRobinPacketQueue::PriorityPacketQueue::const_iterator
RoundRobinPacketQueue::PriorityPacketQueue::begin() const {
  return c.begin();
}

RoundRobinPacketQueue::PriorityPacketQueue::const_iterator
RoundRobinPacketQueue::PriorityPacketQueue::end() const {
  return c.end();
}

RoundRobinPacketQueue::Stream::Stream() : size(webrtc::DataSize::Zero()), ssrc(0) {}
RoundRobinPacketQueue::Stream::Stream(const Stream& stream) = default;
RoundRobinPacketQueue::Stream::~Stream() = default;

//bool IsEnabled(const WebRtcKeyValueConfig* field_trials, const char* name) {
//  if (!field_trials) {
//    return false;
//  }
//  return absl::StartsWith(field_trials->Lookup(name), "Enabled");
//}

RoundRobinPacketQueue::RoundRobinPacketQueue(
	webrtc::Timestamp start_time)//,
   // const WebRtcKeyValueConfig* field_trials)
    : transport_overhead_per_packet_(webrtc::DataSize::Zero()),
      time_last_updated_(start_time),
      paused_(false),
      size_packets_(0),
      size_(webrtc::DataSize::Zero()),
      max_size_(kMaxLeadingSize),
      queue_time_sum_(webrtc::TimeDelta::Zero()),
      pause_time_sum_(webrtc::TimeDelta::Zero()),
      include_overhead_(false) {}

RoundRobinPacketQueue::~RoundRobinPacketQueue() {
  // Make sure to release any packets owned by raw pointer in QueuedPacket.
  while (!Empty()) {
    Pop();
  }
}

void RoundRobinPacketQueue::Push(int priority,
	webrtc::Timestamp enqueue_time,
                                 uint64_t enqueue_order,
                                 std::unique_ptr<RtpPacketToSend> packet) {
  RTC_DCHECK(packet->packet_type().has_value());
  if (size_packets_ == 0) {
    // Single packet fast-path.
    single_packet_queue_.emplace(
        QueuedPacket(priority, enqueue_time, enqueue_order,
                     enqueue_times_.end(), std::move(packet)));
    UpdateQueueTime(enqueue_time);
    single_packet_queue_->SubtractPauseTime(pause_time_sum_);
    size_packets_ = 1;
    size_ += PacketSize(*single_packet_queue_);
  } else {
    MaybePromoteSinglePacketToNormalQueue();
    Push(QueuedPacket(priority, enqueue_time, enqueue_order,
                      enqueue_times_.insert(enqueue_time), std::move(packet)));
  }
}

std::unique_ptr<RtpPacketToSend> RoundRobinPacketQueue::Pop() {

  if (single_packet_queue_.has_value()) {
    RTC_DCHECK(stream_priorities_.empty());
    std::unique_ptr<RtpPacketToSend> rtp_packet(
        single_packet_queue_->RtpPacket());
    single_packet_queue_.reset();
    queue_time_sum_ = webrtc::TimeDelta::Zero();
    size_packets_ = 0;
    size_ = webrtc::DataSize::Zero();
    return rtp_packet;
  }

  RTC_DCHECK(!Empty());
  // 获取优先级最高的流
  Stream* stream = GetHighestPriorityStream();
  const QueuedPacket& queued_packet = stream->packet_queue.top();
  // 一旦数据包从stream的队列中出列，stream的优先级可能会发生变化，需要更新
// 先从map中删除该流的优先级
  stream_priorities_.erase(stream->priority_it);

  // Calculate the total amount of time spent by this packet in the queue
  // while in a non-paused state. Note that the `pause_time_sum_ms_` was
  // subtracted from `packet.enqueue_time_ms` when the packet was pushed, and
  // by subtracting it now we effectively remove the time spent in in the
  // queue while in a paused state.
  webrtc::TimeDelta time_in_non_paused_state =
      time_last_updated_ - queued_packet.EnqueueTime() - pause_time_sum_;
  queue_time_sum_ -= time_in_non_paused_state;

  RTC_CHECK(queued_packet.EnqueueTimeIterator() != enqueue_times_.end());
  enqueue_times_.erase(queued_packet.EnqueueTimeIterator());

  // Update `bytes` of this stream. The general idea is that the stream that
  // has sent the least amount of bytes should have the highest priority.
  // The problem with that is if streams send with different rates, in which
  // case a "budget" will be built up for the stream sending at the lower
  // rate. To avoid building a too large budget we limit `bytes` to be within
  // kMaxLeading bytes of the stream that has sent the most amount of bytes.
  webrtc::DataSize packet_size = PacketSize(queued_packet);
  // 更新stream累计发送的字节数
// 大视频流 2000
// 小视频流 50
// 确保两个流之间累计发送的字节数最大不会相差kMaxLeadingSize
  stream->size =
      std::max(stream->size + packet_size, max_size_ - kMaxLeadingSize);
  max_size_ = std::max(max_size_, stream->size);

  size_ -= packet_size;
  size_packets_ -= 1;
  RTC_CHECK(size_packets_ > 0 || queue_time_sum_ == webrtc::TimeDelta::Zero());

  std::unique_ptr<RtpPacketToSend> rtp_packet(queued_packet.RtpPacket());
  stream->packet_queue.pop();

  // If there are packets left to be sent, schedule the stream again.
  RTC_CHECK(!IsSsrcScheduled(stream->ssrc));
  // 重新计算stream的优先级
  if (stream->packet_queue.empty()) {
    stream->priority_it = stream_priorities_.end();
  } else {
    int priority = stream->packet_queue.top().Priority();
    stream->priority_it = stream_priorities_.emplace(
        StreamPrioKey(priority, stream->size), stream->ssrc);
  }

  return rtp_packet;
}

bool RoundRobinPacketQueue::Empty() const {
  if (size_packets_ == 0) {
    RTC_DCHECK(!single_packet_queue_.has_value() && stream_priorities_.empty());
    return true;
  }
  RTC_DCHECK(single_packet_queue_.has_value() || !stream_priorities_.empty());
  return false;
}

size_t RoundRobinPacketQueue::SizeInPackets() const {
  return size_packets_;
}

webrtc::DataSize RoundRobinPacketQueue::Size() const {
  return size_;
}

absl::optional<webrtc::Timestamp> RoundRobinPacketQueue::LeadingAudioPacketEnqueueTime()
    const {
  if (single_packet_queue_.has_value()) {
    if (single_packet_queue_->Type() == RtpPacketMediaType::kAudio) {
      return single_packet_queue_->EnqueueTime();
    }
    return absl::nullopt;
  }

  if (stream_priorities_.empty()) {
    return absl::nullopt;
  }
  uint32_t ssrc = stream_priorities_.begin()->second;

  const auto& top_packet = streams_.find(ssrc)->second.packet_queue.top();
  if (top_packet.Type() == RtpPacketMediaType::kAudio) {
    return top_packet.EnqueueTime();
  }
  return absl::nullopt;
}

webrtc::Timestamp RoundRobinPacketQueue::OldestEnqueueTime() const {
  if (single_packet_queue_.has_value()) {
    return single_packet_queue_->EnqueueTime();
  }

  if (Empty())
    return webrtc::Timestamp::MinusInfinity();
  RTC_CHECK(!enqueue_times_.empty());
  return *enqueue_times_.begin();
}

void RoundRobinPacketQueue::UpdateQueueTime(webrtc::Timestamp now) {
  RTC_CHECK_GE(now, time_last_updated_);
  if (now == time_last_updated_)
    return;

  webrtc::TimeDelta delta = now - time_last_updated_;

  if (paused_) {
    pause_time_sum_ += delta;
  } else {
    queue_time_sum_ += webrtc::TimeDelta::Micros(delta.us() * size_packets_);
  }

  time_last_updated_ = now;
}

void RoundRobinPacketQueue::SetPauseState(bool paused, webrtc::Timestamp now) {
  if (paused_ == paused)
    return;
  UpdateQueueTime(now);
  paused_ = paused;
}

void RoundRobinPacketQueue::SetIncludeOverhead() {
  MaybePromoteSinglePacketToNormalQueue();
  include_overhead_ = true;
  // We need to update the size to reflect overhead for existing packets.
  for (const auto& stream : streams_) {
    for (const QueuedPacket& packet : stream.second.packet_queue) {
      size_ += webrtc::DataSize::Bytes(packet.RtpPacket()->headers_size()) +
               transport_overhead_per_packet_;
    }
  }
}

void RoundRobinPacketQueue::SetTransportOverhead(webrtc::DataSize overhead_per_packet) {
  MaybePromoteSinglePacketToNormalQueue();
  if (include_overhead_) {
	  webrtc::DataSize previous_overhead = transport_overhead_per_packet_;
    // We need to update the size to reflect overhead for existing packets.
    for (const auto& stream : streams_) {
      int packets = stream.second.packet_queue.size();
      size_ -= packets * previous_overhead;
      size_ += packets * overhead_per_packet;
    }
  }
  transport_overhead_per_packet_ = overhead_per_packet;
}

webrtc::TimeDelta RoundRobinPacketQueue::AverageQueueTime() const {
  if (Empty())
    return webrtc::TimeDelta::Zero();
  return queue_time_sum_ / size_packets_;
}

void RoundRobinPacketQueue::Push(QueuedPacket packet) 
{
	// 查找packet是否存在对应的stream
  auto stream_info_it = streams_.find(packet.Ssrc());
  if (stream_info_it == streams_.end()) {// Stream第一个数据包
    stream_info_it = streams_.emplace(packet.Ssrc(), Stream()).first;
	// 新的数据尚未列入优先级排序
    stream_info_it->second.priority_it = stream_priorities_.end();
    stream_info_it->second.ssrc = packet.Ssrc();
  }

  Stream* stream = &stream_info_it->second;
  // 调整流的优先级
  if (stream->priority_it == stream_priorities_.end()) {
    // If the SSRC is not currently scheduled, add it to `stream_priorities_`.
	  // 在map当中还没有进行该流的优先级排序
    RTC_CHECK(!IsSsrcScheduled(stream->ssrc));
    stream->priority_it = stream_priorities_.emplace(
        StreamPrioKey(packet.Priority(), stream->size), packet.Ssrc());
  } else if (packet.Priority() < stream->priority_it->first.priority) {
    // If the priority of this SSRC increased, remove the outdated StreamPrioKey
    // and insert a new one with the new priority. Note that `priority_` uses
    // lower ordinal for higher priority.

    stream_priorities_.erase(stream->priority_it);
	// 重新插入，更新优先级
    stream->priority_it = stream_priorities_.emplace(
        StreamPrioKey(packet.Priority(), stream->size), packet.Ssrc());
  }
  RTC_CHECK(stream->priority_it != stream_priorities_.end());

  if (packet.EnqueueTimeIterator() == enqueue_times_.end()) {
    // Promotion from single-packet queue. Just add to enqueue times.
    packet.UpdateEnqueueTimeIterator(
        enqueue_times_.insert(packet.EnqueueTime()));
  } else {
    // In order to figure out how much time a packet has spent in the queue
    // while not in a paused state, we subtract the total amount of time the
    // queue has been paused so far, and when the packet is popped we subtract
    // the total amount of time the queue has been paused at that moment. This
    // way we subtract the total amount of time the packet has spent in the
    // queue while in a paused state.
    UpdateQueueTime(packet.EnqueueTime());
    packet.SubtractPauseTime(pause_time_sum_);

    size_packets_ += 1;
    size_ += PacketSize(packet);
  }

  stream->packet_queue.push(packet);
}

webrtc::DataSize RoundRobinPacketQueue::PacketSize(const QueuedPacket& packet) const {
	webrtc::DataSize packet_size = webrtc::DataSize::Bytes(packet.RtpPacket()->payload_size() +
                                         packet.RtpPacket()->padding_size());
  if (include_overhead_) {
    packet_size += webrtc::DataSize::Bytes(packet.RtpPacket()->headers_size()) +
                   transport_overhead_per_packet_;
  }
  return packet_size;
}

void RoundRobinPacketQueue::MaybePromoteSinglePacketToNormalQueue() {
  if (single_packet_queue_.has_value()) {
    Push(*single_packet_queue_);
    single_packet_queue_.reset();
  }
}

RoundRobinPacketQueue::Stream*
RoundRobinPacketQueue::GetHighestPriorityStream() {
  RTC_CHECK(!stream_priorities_.empty());
  uint32_t ssrc = stream_priorities_.begin()->second;

  auto stream_info_it = streams_.find(ssrc);
  RTC_CHECK(stream_info_it != streams_.end());
  RTC_CHECK(stream_info_it->second.priority_it == stream_priorities_.begin());
  RTC_CHECK(!stream_info_it->second.packet_queue.empty());
  return &stream_info_it->second;
}

bool RoundRobinPacketQueue::IsSsrcScheduled(uint32_t ssrc) const {
  for (const auto& scheduled_stream : stream_priorities_) {
    if (scheduled_stream.second == ssrc)
      return true;
  }
  return false;
}

}  // namespace webrtc

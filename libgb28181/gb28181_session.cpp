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
				   date:  2025-10-08



 ******************************************************************************/
#include "libmedia_transfer_protocol/libgb28181/gb28181_session.h"
#include "rtc_base/internal/default_socket_server.h"
#include "libice/stun.h"
#include "rtc_base/third_party/base64/base64.h"
#include "rtc_base/message_digest.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_util.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/common_header.h"
#include "rtc_base/string_encode.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
#include "rtc_base/logging.h"
namespace libmedia_transfer_protocol
{
	namespace libgb28181
	{
		Gb28181Session::Gb28181Session(rtc::Socket * socket, rtc::Thread * work_thread)
			: socket_(socket)
			, work_thread_(work_thread)
			
			, video_receive_stream_(nullptr)
			, rtp_stream_receive_controller_(nullptr)
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
		{
			InitSocketSignals();
		}
		Gb28181Session::~Gb28181Session()
		{
		}
		void Gb28181Session::InitSocketSignals()
		{
			socket_->SignalCloseEvent.connect(this, &Gb28181Session::OnClose);
			socket_->SignalConnectEvent.connect(this, &Gb28181Session::OnConnect);
			socket_->SignalReadEvent.connect(this, &Gb28181Session::OnRead);
			socket_->SignalWriteEvent.connect(this, &Gb28181Session::OnWrite);
		}
		void Gb28181Session::OnConnect(rtc::Socket* socket)
		{
			RTC_LOG_F(LS_INFO) << "";
		}
		void Gb28181Session::OnClose(rtc::Socket* socket, int ret)
		{
			RTC_LOG_F(LS_INFO) << "";
		}
		void Gb28181Session::OnRead(rtc::Socket* socket)
		{
			RTC_LOG_F(LS_INFO) << "";

			rtc::Buffer buffer(1024 * 1024 *8 );

			//recv_buffer_size_ = read_bytes - paser_size;
			rtc::ArrayView<uint8_t> array_buffer(buffer.begin(), buffer.capacity());
			int32_t  read_bytes = 0;
			if (recv_buffer_size_ > 0)
			{
				memcpy((char *)buffer.begin(), recv_buffer_.begin(), recv_buffer_size_);
				read_bytes = recv_buffer_size_;
				recv_buffer_size_ = 0;
			}
			do {
				int bytes = socket->Recv(buffer.begin() + read_bytes, buffer.capacity()- read_bytes, nullptr);
				if (bytes <= 0)
					break;
				read_bytes += bytes;
				if (read_bytes > (buffer.capacity() / 2))
				{
					break;
				}
			} while (true);

			int32_t   parse_size = 0;
			while (read_bytes - parse_size > 2) 
			{
				//ntohs(*(int16_t*)(buffer.begin()[parse_size]));//ntohs(*(int16_t*)(ptroc)); 
				//uint8_t  * ptroc = buffer.begin() + parse_size;
				int16_t  payload_size =   libmedia_transfer_protocol::ByteReader<int16_t>::ReadBigEndian((&buffer.begin()[parse_size]));
				parse_size += 2;
				//parse_size += payload_size;
				RTC_LOG(LS_INFO) << "payload_size: " << payload_size;
				//printf("payload_size:%u\n",   payload_size);

#if 1
				if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + parse_size, payload_size)))
				{
					RtpPacketReceived  rtp_packet_received;

					//libmedia_transfer_protocol::RtpPacket  rtp_packet;
					//bool ret = rtp_packet.Parse(buffer.begin() + paser_size, rtsp_magic.length_/*read_bytes - paser_size*/);
					bool ret = rtp_packet_received.Parse(buffer.begin() + parse_size, payload_size);
					if (!ret)
					{

						RTC_LOG(LS_WARNING) << "rtp parse failed !!! size:" << (read_bytes - parse_size); //<< "  , hex :" << rtc::hex_encode((const char *)(buffer.begin() + paser_size), (size_t)(read_bytes - paser_size));
					}
					else
					{
						RTC_LOG(LS_INFO) << "rtp info :" << rtp_packet_received.ToString();
						if (rtp_packet_received.PayloadType() == 96)
						{
							

							static FILE *out_file_ptr = fopen("test_ps.ts", "wb+");
							if (out_file_ptr)
							{
								fwrite(rtp_packet_received.payload().data(), 1, rtp_packet_received.payload_size(), out_file_ptr);
							
								fflush(out_file_ptr);
							}

							if (!video_receive_stream_)
							{
								work_thread_->PostTask(RTC_FROM_HERE, [this, ssrc = rtp_packet_received.Ssrc()]() {
									video_receive_stream_ = std::make_unique<VideoReceiveStream>();
									video_receive_stream_->RegisterDecodeCompleteCallback(callback_);
									video_receive_stream_->init(libmedia_codec::VideoCodecType::kVideoCodecH264, 1280, 720);
									rtp_stream_receive_controller_ = std::make_unique<libmedia_transfer_protocol::RtpStreamReceiverController>();
									//;
									rtp_stream_receive_controller_->AddSink(ssrc, video_receive_stream_.get());
								});
							}
							work_thread_->PostTask(RTC_FROM_HERE, [this,  packet = std::move(rtp_packet_received)]() {
								rtp_stream_receive_controller_->OnRtpPacket(packet);
							});

						}
						parse_size += payload_size;
					}
				}
				else if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + parse_size, payload_size/*read_bytes - paser_size*/)))
				{
					libmedia_transfer_protocol::rtcp::CommonHeader rtcp_block;  //rtcp_packet;
					bool ret = rtcp_block.Parse(buffer.begin() + parse_size, payload_size/* read_bytes - paser_size*/);
					if (!ret)
					{
						RTC_LOG(LS_WARNING) << "rtcp parse failed !!!";
					}
					else
					{
						parse_size += payload_size;
						//	RTC_LOG(LS_INFO) << "rtcp info :" << rtcp_block.ToString();
					}
				}
				else
				{
					RTC_LOG(LS_ERROR) << " not know type --> : payload_size: " << payload_size;
					parse_size += payload_size;
				}
#endif 
			}
			if (read_bytes - parse_size > 0)
			{
				memcpy((char *)recv_buffer_.begin(), buffer.begin() + parse_size, (read_bytes - parse_size));
				recv_buffer_size_ = read_bytes - parse_size;
			}
			else
			{
				//memcpy((char *)recv_buffer_.begin(), buffer.begin() + parse_size, (read_bytes - parse_size));
				//recv_buffer_size_ = read_bytes - parse_size;
			}
			return;
		}
		void Gb28181Session::OnWrite(rtc::Socket* socket)
		{
			RTC_LOG_F(LS_INFO) << "";
		}
	}
}
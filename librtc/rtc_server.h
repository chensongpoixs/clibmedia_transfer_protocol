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
				   date:  2025-10-14



 ******************************************************************************/


#ifndef _C_RTC_SERVER_H_
#define _C_RTC_SERVER_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <string>
#include "rtc_base/buffer.h"
#include "rtc_base/thread.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#ifdef WIN32
#include "rtc_base/win32_socket_server.h"
#include <vcruntime.h>
#endif
#include "rtc_base/async_udp_socket.h"
#include "libmedia_transfer_protocol/transport.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
namespace libmedia_transfer_protocol {
	namespace librtc {
		
		class RtcServer : public sigslot::has_slots<>
		{
		public:
			explicit RtcServer();
			virtual ~RtcServer();



		public:
			
			bool Start(const char * ip, uint16_t port);

		public:
			int SendPacket(const rtc::Buffer& packet, const rtc::PacketOptions& options);
			int SendPacketTo(const rtc::Buffer& packet,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);


			// rtp 
			int SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			int32_t SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
				const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			// rtcp 
			int SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);

		public:

			void  OnRecvPacket(rtc::AsyncPacketSocket * socket, const uint8_t * data, size_t len,
				const rtc::SocketAddress & addr, const int64_t & ms);
			 
		public:
			rtc::Thread* signaling_thread() { return udp_server_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return udp_server_->signaling_thread(); }
			rtc::Thread* worker_thread() { return udp_server_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return udp_server_->worker_thread(); }
			rtc::Thread* network_thread() { return udp_server_->network_thread(); }
			const rtc::Thread* network_thread() const { return udp_server_->network_thread(); }
			 
		public:
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalStunPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalDtlsPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtpPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtcpPacket;



		 
		public:
			
			bool IsStun(const uint8_t  * data, int32_t len);
			bool IsDtls(const uint8_t * data, int32_t len);
			bool IsRtp(const uint8_t * data, int32_t len);
			bool IsRtcp(const uint8_t * data, int32_t len);
		 
		private:


			std::unique_ptr<libnetwork::UdpServer>      udp_server_;

		};

	}
}

#endif // 
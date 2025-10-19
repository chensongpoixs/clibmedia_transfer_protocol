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
namespace libmedia_transfer_protocol {
	namespace librtc {
		
		class RtcServer : public sigslot::has_slots<>
		{
		public:
			RtcServer(rtc::Thread* network);
			~RtcServer();



		public:
			
			void Start(const char * ip, uint16_t port);

			  int SendPacket(const rtc::Buffer& packet, const rtc::PacketOptions& options)  ;
			  int SendPacketTo(const rtc::Buffer& packet,
				  const rtc::SocketAddress& addr,
				  const rtc::PacketOptions& options);


			  // rtp 
			  int SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			  int32_t SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets, 
				  const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			  // rtcp 
			  int SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			//void SendPacket();
		public:
			//void OnConnect(rtc::Socket*socket);
			//void OnRead(rtc::Socket* socket);
			//void OnWrite(rtc::Socket* socket);
			//void OnClose(rtc::Socket* socket, int32_t );

			void  OnRecvPacket(rtc::AsyncPacketSocket* socket,
				const char* data,
				size_t len,
				const rtc::SocketAddress& addr,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t& ms);

			void OnSend(rtc::AsyncPacketSocket* socket);
		public:
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const char*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalStunPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const char*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalDtlsPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const char*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtpPacket;
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const char*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtcpPacket;



		public:
			sigslot::signal4<rtc::Socket*,
				const rtc::Buffer&,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalStunPacketBuffer;
			sigslot::signal4<rtc::Socket*,
				const rtc::Buffer&,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalDtlsPacketBuffer;
			sigslot::signal4<rtc::Socket*,
				const rtc::Buffer&,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtpPacketBuffer;
			sigslot::signal4<rtc::Socket*,
				const rtc::Buffer&,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtcpPacketBuffer;
		public:
			
			bool IsStun(const char * data, int32_t len);
			bool IsDtls(const char * data, int32_t len);
			bool IsRtp(const char * data, int32_t len);
			bool IsRtcp(const char * data, int32_t len);
		public:
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
		private:
			void InitSocketSignals();
		private:


			rtc::Thread*   network_;

			rtc::SocketAddress               server_address_;
#if 1
			std::unique_ptr<rtc::AsyncPacketSocket>      udp_control_socket_;
#else 
			std::unique_ptr<rtc::Socket>       control_socket_;
#endif 
		};

	}
}

#endif // 
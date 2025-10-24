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
				   date:  2025-10-24



 ******************************************************************************/


#ifndef _C_LIBNETWORK_UDP_SERVER_H_
#define _C_LIBNETWORK_UDP_SERVER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "rtc_base/buffer.h"
#include "rtc_base/thread.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"


namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		

		class UdpServer : public   sigslot::has_slots<>
		{
		public:

			explicit UdpServer();

			virtual ~UdpServer();
		public:
		 
			bool Startup(const std::string &ip, uint16_t port);
		public:
			//int SendPacket(const rtc::Buffer& packet, const rtc::PacketOptions& options);
			int32_t SendPacketTo(const rtc::Buffer& packet,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);
			int32_t   SendTo(const uint8_t* pv,
				size_t cb,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);

			// rtp 
			int32_t SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			int32_t SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
				const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			// rtcp 
			int32_t SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
		public:
			// Emitted each time a packet is read. Used only for UDP and
			// connected TCP sockets.
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalReadPacket;

		public:

			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }
			
		public:

			void InitSocketSignals();
			
			void OnNewConnection(rtc::AsyncPacketSocket* socket1, rtc::AsyncPacketSocket* socket2);
			void OnConnect(rtc::AsyncPacketSocket* socket);
			void OnRecvPacket(rtc::AsyncPacketSocket * socket, const char * data, size_t len,
				const rtc::SocketAddress & addr, const int64_t & ms);
			void OnAddressReady(rtc::AsyncPacketSocket* socket, const rtc::SocketAddress&addr);
			void OnSend(rtc::AsyncPacketSocket* socket);
			void OnClose(rtc::AsyncPacketSocket* socket, int32_t);
		public:
		private:
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;
			rtc::SocketAddress               server_address_;
			std::unique_ptr<rtc::AsyncPacketSocket>      udp_control_socket_;
 
		};
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_
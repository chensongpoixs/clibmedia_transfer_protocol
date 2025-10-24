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

#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/buffer.h"
#include "rtc_base/byte_buffer.h"

namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		UdpServer::UdpServer()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
		{
		}
		UdpServer::~UdpServer()
		{
		}
		bool UdpServer::Startup(const std::string & ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);
			if (network_thread()->IsCurrent())
			{ 
				udp_control_socket_.reset(rtc::AsyncUDPSocket::Create(network_thread()->socketserver(), server_address_));
				//control_socket_.reset(network_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
				if (!udp_control_socket_)
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					return  false;
				}
				InitSocketSignals(); 
				LIBNETWORK_LOG(LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
				return true;
			}
			 
			 
			return 	network_thread()->Invoke<bool>(RTC_FROM_HERE, [this]() {
					udp_control_socket_.reset(rtc::AsyncUDPSocket::Create(network_thread()->socketserver(), server_address_));
					// control_socket_.reset(network_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
					if (!udp_control_socket_)
					{
						LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
						return false;
					}
					InitSocketSignals();
					 
					LIBNETWORK_LOG (LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
					return true;
				});
			 
		}
		void UdpServer::InitSocketSignals()
		{ 
			udp_control_socket_->SignalNewConnection.connect(this, &UdpServer::OnNewConnection);
			udp_control_socket_->SignalConnect.connect(this, &UdpServer::OnConnect);
			udp_control_socket_->SignalAddressReady.connect(this, &UdpServer::OnAddressReady);
			
			udp_control_socket_->SignalReadPacket.connect(this, &UdpServer::OnRecvPacket);
			udp_control_socket_->SignalReadyToSend.connect(this, &UdpServer::OnSend);
			udp_control_socket_->SignalClose.connect(this, &UdpServer::OnClose);
 
		}

		int32_t UdpServer::SendPacketTo(const rtc::Buffer& packet,
			const rtc::SocketAddress& addr,
			const rtc::PacketOptions& options)
		{
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
		}

		int32_t UdpServer::SendTo(const uint8_t * pv, size_t cb, const rtc::SocketAddress & addr, const rtc::PacketOptions & options)
		{
			return udp_control_socket_->SendTo(pv, cb, addr, options);
		}


		// rtp 
		int32_t UdpServer::SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
		}
		int32_t UdpServer::SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
			const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			for (auto &  p : packets)
			{
				udp_control_socket_->SendTo(p->data(), p->size(), addr, options);
			}
			return 0;
			
		}
		// rtcp 
		int32_t UdpServer::SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
		}
		
		void UdpServer::OnNewConnection(rtc::AsyncPacketSocket * socket1, rtc::AsyncPacketSocket * socket2)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}

		void  UdpServer::OnConnect(rtc::AsyncPacketSocket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		void UdpServer::OnAddressReady(rtc::AsyncPacketSocket* socket, const rtc::SocketAddress&addr)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "addr:" << socket->GetRemoteAddress().ToString();
		}
		void UdpServer::OnRecvPacket(rtc::AsyncPacketSocket * socket, const char  * data, size_t len,
			const rtc::SocketAddress & addr, const int64_t & ms)
		{
			//LIBNETWORK_LOG_T_F(LS_INFO) << "";
			SignalReadPacket(socket, (const uint8_t *)data, len, addr, ms);
		}
		
		void UdpServer::OnSend(rtc::AsyncPacketSocket * socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "addr:" << socket->GetRemoteAddress().ToString();
		}
		void  UdpServer::OnClose(rtc::AsyncPacketSocket* socket, int32_t)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
	}

}

 
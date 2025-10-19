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
				   date:  2025-10-11



 ******************************************************************************/
#include "libmedia_transfer_protocol/libsip/udp_sip_server.h"
#include "rtc_base/logging.h"
namespace libmedia_transfer_protocol
{
	namespace libsip
	{
		UdpSipServer::UdpSipServer(rtc::Thread* network_thread, rtc::Thread* work_thread)
		: network_thread_(network_thread)
		, work_thread_(work_thread)
		, control_socket_(nullptr){}
		UdpSipServer::~UdpSipServer(){}
	 

		bool UdpSipServer::Start(const char *ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);

			control_socket_.reset(network_thread_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
			if (!control_socket_)
			{
				RTC_LOG(LS_WARNING) << "create socket failed !!! " << server_address_.ToString();
				return false;
			}
			InitSocketSignals();
			int32_t ret = control_socket_->Bind(server_address_);
			if (ret != 0)
			{
				RTC_LOG(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
				return false;
			}

			ret = control_socket_->Listen(500);
			if (ret != 0)
			{
				RTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				return false;
			}
			return true;
		}
	 

		void UdpSipServer::InitSocketSignals()
		{
			control_socket_->SignalCloseEvent.connect(this, &UdpSipServer::OnClose);
			control_socket_->SignalConnectEvent.connect(this, &UdpSipServer::OnConnect);
			control_socket_->SignalReadEvent.connect(this, &UdpSipServer::OnRead);
			control_socket_->SignalWriteEvent.connect(this, &UdpSipServer::OnWrite);
		}
		void UdpSipServer::OnConnect(rtc::Socket* socket)
		{
		
			RTC_LOG_F(LS_INFO) << "";
		}
		void UdpSipServer::OnClose(rtc::Socket* socket, int ret) 
		{
			RTC_LOG_F(LS_INFO) << "";
		}
		void UdpSipServer::OnRead(rtc::Socket* socket) 
		{
			//RTC_LOG_F(LS_INFO) << "";
			// UDP // mtu 1500
			rtc::Buffer buffer(2000);
			buffer.SetSize(0);
			rtc::SocketAddress out_addr;
			int64_t timestamp = 0;
			do {
				
					
				int bytes = socket->RecvFrom(buffer.begin() + buffer.size(), buffer.capacity() - buffer.size(), &out_addr, &timestamp);
				if (bytes <= 0)
				{
					break;
				}
				buffer.SetSize(buffer.size() +  bytes);
				if (buffer.size() >= (buffer.capacity()))
				{
					break;
				}
			} while (true);

			RTC_LOG(LS_INFO) << "recvFrom : " << out_addr.ToString() << ",  data => " << std::string((char *)buffer.data(), buffer.size());
		}
		void UdpSipServer::OnWrite(rtc::Socket* socket) 
		{
			RTC_LOG_F(LS_INFO) << "";
		}
	}
}
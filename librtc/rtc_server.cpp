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
#include "libmedia_transfer_protocol/librtc/rtc_server.h"
#include "rtc_base/logging.h"
#include "rtc_base/async_udp_socket.h"

namespace libmedia_transfer_protocol {
	namespace librtc {
		 RtcServer::RtcServer(rtc::Thread * network)
			 :network_(network)
		{
		}
		 RtcServer::~RtcServer()
		 {
			 if (network_->IsCurrent())
			 {
#if 1
				 //udp_control_socket_->SignalCloseEvent.disconnect(this);
				 //udp_control_socket_->SignalConnectEvent.disconnect(this);
				 udp_control_socket_->SignalReadyToSend.disconnect(this);
				 udp_control_socket_->SignalReadPacket.disconnect(this);
#else 
				 control_socket_->SignalCloseEvent.disconnect(this);
				 control_socket_->SignalConnectEvent.disconnect(this);
				 control_socket_->SignalReadEvent.disconnect(this);
				 control_socket_->SignalWriteEvent.disconnect(this);
#endif // 
			 }
			 else
			 {
				 network_->PostTask(RTC_FROM_HERE, [ this,  socket = std::move(udp_control_socket_) ]() {
					
#if 0
					 control_socket_->SignalCloseEvent.disconnect(this);
					 control_socket_->SignalConnectEvent.disconnect(this);
					 control_socket_->SignalReadEvent.disconnect(this);
					 control_socket_->SignalWriteEvent.disconnect(this);
#else 
					 socket->SignalReadPacket.disconnect(this);
				     socket->SignalReadyToSend.disconnect(this);
					//socket->SignalReadEvent.disconnect(this);
					//socket->SignalWriteEvent.disconnect(this);
#endif // 
				 });
			 }

		 }

		 void RtcServer::Start(const char * ip, uint16_t port)
		 {
			 server_address_.SetIP(ip);
			 server_address_.SetPort(port);
			 if (network_->IsCurrent())
			 {


				  udp_control_socket_.reset(  rtc::AsyncUDPSocket::Create(network_->socketserver(), server_address_));
				 //control_socket_.reset(network_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
				 if (!udp_control_socket_)
				 {
					 LIBRTC_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					 return;
				 }
				 InitSocketSignals();
				 /*int32_t ret = control_socket_->Bind(server_address_);
				 if (ret != 0)
				 {
					 LIBRTC_LOG(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
					 return;
				 }

				 ret = control_socket_->Listen(5);
				 if (ret != 0)
				 {
					 LIBRTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
					 return;
				 }*/
				 LIBRTC_LOG(LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
			 }
			 else
			 {
				 network_->Invoke<void>(RTC_FROM_HERE, [this]() {
					  udp_control_socket_.reset(  rtc::AsyncUDPSocket::Create(network_->socketserver(), server_address_));
					// control_socket_.reset(network_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
					 if (!udp_control_socket_)
					 {
						 LIBRTC_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
						 return;
					 }
					 InitSocketSignals();
					/* int32_t ret = control_socket_->Bind(server_address_);
					 if (ret != 0)
					 {
						 LIBRTC_LOG(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
						 return;
					 }

					 ret = control_socket_->Listen(5);
					 if (ret != 0)
					 {
						 LIBRTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
						 return;
					 }*/
					 LIBRTC_LOG(LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
				 });
			 }
			 return  ;
		 }
		 void RtcServer::OnRecvPacket(rtc::AsyncPacketSocket * socket, const char * data, size_t len, const rtc::SocketAddress & addr, const int64_t & ms)
		 {
			 LIBRTC_LOG_T_F(LS_INFO) << "";
			 if (IsStun(data, len))
			 {
				 SignalStunPacket(socket, data, len, addr, ms);
			 }
			 else if (IsDtls(data, len))
			 {
				 SignalDtlsPacket(socket, data, len, addr, ms);
			 }
			 else if (IsRtp(data, len))
			 {
				 SignalRtpPacket(socket, data, len, addr, ms);
			 }
			 else if (IsRtcp(data, len))
			 {
				 SignalRtcpPacket(socket, data, len, addr, ms);
			 }
			 else
			 {
				 LIBRTC_LOG_T_F(LS_WARNING) << " recv unk type packet addr:" << addr.ToString();
			 }
			 //SignalReadPacket(socket, data, len, addr, ms);
		 }
		 void RtcServer::OnSend(rtc::AsyncPacketSocket * socket)
		 {
			 LIBRTC_LOG_T_F(LS_INFO) << "addr:" << socket->GetRemoteAddress().ToString();
		 }
		 bool RtcServer::IsStun(const char * data, int32_t len)
		 {
			 return len >= 20 && data[0] >= 0 && data[0] <= 3;
		 }
		 bool RtcServer::IsDtls(const char * data, int32_t len)
		 {
			 return len >= 13 && data[0] >= 20 && data[0] <= 63;
		 }
		 bool RtcServer::IsRtp(const char * data, int32_t len)
		 {
			 uint8_t pt = (uint8_t)data[1];
			 return len >= 12 && data[0] & 0x80 && !(pt >= 192 && pt <= 223);
		 }
		 bool RtcServer::IsRtcp(const char * data, int32_t len)
		 {
			 uint8_t pt = (uint8_t)data[1];
			 return len >= 12 && data[0] & 0x80 && (pt >= 192 && pt <= 223);
		 }
		 void RtcServer::InitSocketSignals()
		 {
			

#if 1
			 udp_control_socket_->SignalReadPacket.connect(this, &RtcServer::OnRecvPacket);
			 udp_control_socket_->SignalReadyToSend.connect(this, &RtcServer::OnSend);
#else 

			  control_socket_->SignalCloseEvent.connect(this, &RtcServer::OnClose);
			  control_socket_->SignalConnectEvent.connect(this, &RtcServer::OnConnect);
			  control_socket_->SignalReadEvent.connect(this, &RtcServer::OnRead);
			  control_socket_->SignalWriteEvent.connect(this, &RtcServer::OnWrite);
#endif 
			// udp_control_socket_->SignalReadEvent.connect(this, &RtcServer::OnRead);
			// udp_control_socket_->SignalWriteEvent.connect(this, &RtcServer::OnWrite);
		 }
		// void RtcServer::OnConnect(rtc::Socket * socket)
		// {
		//	 LIBRTC_LOG_T_F(LS_INFO) << "";
		// }
		// void RtcServer::OnRead(rtc::Socket * socket)
		// {
		//	 LIBRTC_LOG_T_F(LS_INFO);
		// }
		//
		// void RtcServer::OnWrite(rtc::Socket * socket)
		// {
		//	 LIBRTC_LOG_T_F(LS_INFO);
		// }
		//
		// void RtcServer::OnClose(rtc::Socket * socket, int32_t)
		// {
		//	 LIBRTC_LOG_T_F(LS_INFO);
		// }


		 void RtcServer::OnConnect(rtc::Socket* socket)
		 {

			 LIBRTC_LOG_T_F(LS_INFO) << "";
		 }
		 void RtcServer::OnClose(rtc::Socket* socket, int ret)
		 {
			 LIBRTC_LOG_T_F(LS_INFO) << "";
		 }
		 void RtcServer::OnRead(rtc::Socket* socket)
		 {
			 LIBRTC_LOG_T_F(LS_INFO) << "";
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
				 buffer.SetSize(buffer.size() + bytes);
				 if (buffer.size() >= (buffer.capacity()))
				 {
					 break;
				 }
			 } while (true);

			 if (IsStun((const char *)buffer.data(), buffer.size()))
			 {
				 SignalStunPacketBuffer(socket, buffer, out_addr, timestamp);
			 }
			 else if (IsDtls((const char *)buffer.data(), buffer.size()))
			 {
				 SignalDtlsPacketBuffer(socket, buffer, out_addr, timestamp);
			 }
			 else if (IsRtp((const char *)buffer.data(), buffer.size()))
			 {
				 SignalRtpPacketBuffer(socket, buffer, out_addr, timestamp);
			 }
			 else if (IsRtcp((const char *)buffer.data(), buffer.size()))
			 {
				 SignalRtcpPacketBuffer(socket, buffer, out_addr, timestamp);
			 }
			 else
			 {
				 LIBRTC_LOG_T_F(LS_WARNING) << " recv unk type packet addr:" << out_addr.ToString();
			 }
			 //RTC_LOG(LS_INFO) << "recvFrom : " << out_addr.ToString() << ",  data => " << std::string((char *)buffer.data(), buffer.size());
		 }
		 void RtcServer::OnWrite(rtc::Socket* socket)
		 {
			 LIBRTC_LOG_T_F(LS_INFO) << "";
		 }

	}
}

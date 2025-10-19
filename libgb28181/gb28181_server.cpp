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
#include "libmedia_transfer_protocol/libgb28181/gb28181_server.h"
#include "rtc_base/logging.h"
namespace  libmedia_transfer_protocol {
	namespace libgb28181
	{



		Gb28181Server::Gb28181Server()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
			, gb28181_sessions_()
			//, audio_play_(nullptr)
		{

			//context_->worker_thread()->PostTask([this]() {
			//	audio_play_ = std::make_unique<libcross_platform_collection_render::AudioCapture>(context_->worker_thread());
			//});
		}

		Gb28181Server::~Gb28181Server()
		{
			if (control_socket_)
			{
				control_socket_->SignalCloseEvent.disconnect(this);
				control_socket_->SignalConnectEvent.disconnect(this);
				control_socket_->SignalReadEvent.disconnect(this);
				control_socket_->SignalWriteEvent.disconnect(this);
				control_socket_.reset();
			}
			if (context_)
			{
				//context_
			}
		}

		bool Gb28181Server::Startup(const std::string &ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);

			control_socket_.reset(context_->network_thread()->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_STREAM));
			if (!context_)
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

			  ret = control_socket_->Listen(5);
			if (ret != 0)
			{
				RTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				return false;
			}
			return true;
		}
		void Gb28181Server::InitSocketSignals()
		{
			control_socket_->SignalCloseEvent.connect(this, &Gb28181Server::OnClose);
			control_socket_->SignalConnectEvent.connect(this, &Gb28181Server::OnConnect);
			control_socket_->SignalReadEvent.connect(this, &Gb28181Server::OnRead);
			control_socket_->SignalWriteEvent.connect(this, &Gb28181Server::OnWrite);
		}
		void Gb28181Server::OnConnect(rtc::Socket* socket)
		{
			RTC_LOG(LS_INFO) << "";
		}
		void Gb28181Server::OnClose(rtc::Socket* socket, int ret)
		{
			RTC_LOG(LS_INFO) << "";
		}
		void Gb28181Server::OnRead(rtc::Socket* socket)
		{
			RTC_LOG(LS_INFO) << "";


			rtc::SocketAddress address;
			rtc::Socket*  client = socket->Accept(&address);
			if (!client)
			{
				RTC_LOG(LS_ERROR) << "accept failed !!!";
				return;
			}
			RTC_LOG(LS_INFO) << "gb28181 new client accept :  " << address.ToString();
			std::unique_ptr<libgb28181::Gb28181Session>  gb28181_session = std::make_unique<libgb28181::Gb28181Session>(client, context_->worker_thread());
			gb28181_session->RegisterDecodeCompleteCallback(callback_);
			gb28181_sessions_.insert(std::make_pair(client, std::move(gb28181_session)));
		}
		void Gb28181Server::OnWrite(rtc::Socket* socket)
		{
			RTC_LOG(LS_INFO) << "";
			
		}
	}

	
}


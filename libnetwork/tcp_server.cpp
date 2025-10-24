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
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/libnetwork/connection.h"
namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{



		TcpServer::TcpServer()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
			, tcp_sessions_()
			//, audio_play_(nullptr)
		{

			//context_->worker_thread()->PostTask([this]() {
			//	audio_play_ = std::make_unique<libcross_platform_collection_render::AudioCapture>(context_->worker_thread());
			//});
		}

		TcpServer::~TcpServer()
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

		bool TcpServer::Startup(const std::string &ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);

			control_socket_.reset(context_->network_thread()->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_STREAM));
			if (!context_)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "create socket failed !!! " << server_address_.ToString();
				return false;
			}
			InitSocketSignals();
			int32_t ret = control_socket_->Bind(server_address_);
			if (ret != 0)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
				return false;
			}

			ret = control_socket_->Listen(500);
			if (ret != 0)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				return false;
			}
			LIBNETWORK_LOG_T_F(LS_INFO) << " tcp start port:" << port << " OK !!!";
			return true;
		}


		void TcpServer::CloseSession(Connection *conn)
		{
			conn->Close();
		}
		void TcpServer::Close(rtc::Socket *socket)
		{
			socket->Close();
		}
		void TcpServer::SetContext(int type, const std::shared_ptr<void> &context)
		{
			contexts_[type] = context;
		}
		void TcpServer::SetContext(int type, std::shared_ptr<void> &&context)
		{
			contexts_[type] = std::move(context);
		}
		void TcpServer::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		void TcpServer::ClearContext()
		{
			contexts_.clear();
		}
		void TcpServer::OnSessionRecv(Connection * conn, const rtc::CopyOnWriteBuffer & data)
		{
			SignalOnRecv(conn, data);
		}
		void TcpServer::OnSessionClose(Connection*  conn)
		{
			network_thread()->PostTask(RTC_FROM_HERE, [this, conn]() {
				//LIBTCP_LOG(LS_INFO) << "";
				LIBNETWORK_LOG_T_F(LS_INFO) << "";
				auto iter = tcp_sessions_.find(conn->GetSocket());
				if (iter == tcp_sessions_.end())
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << " tcp session not find socket :" << conn->GetSocket()->GetRemoteAddress().ToString();
					SignalOnDestory(conn);
					conn->SignalOnClose.disconnect_all();
					conn->SignalOnRecv.disconnect_all();
					return;
				}
				SignalOnDestory(iter->second.get());
				conn->SignalOnClose.disconnect_all();
				conn->SignalOnRecv.disconnect_all();
				 
				iter->second.reset();
				tcp_sessions_.erase(iter);
			});
			
		}
		void TcpServer::InitSocketSignals()
		{
			control_socket_->SignalCloseEvent.connect(this, &TcpServer::OnClose);
			control_socket_->SignalConnectEvent.connect(this, &TcpServer::OnConnect);
			control_socket_->SignalReadEvent.connect(this, &TcpServer::OnRead);
			control_socket_->SignalWriteEvent.connect(this, &TcpServer::OnWrite);
		}
		void TcpServer::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		void TcpServer::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			auto iter = tcp_sessions_.find(socket);
			if (iter == tcp_sessions_.end())
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << " tcp session not find socket :" << socket->GetRemoteAddress().ToString();

				return;
			}
			SignalOnDestory(iter->second.get());
			iter->second.reset();
			tcp_sessions_.erase(iter);
		}
		void TcpServer::OnRead(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";


			rtc::SocketAddress address;
			rtc::Socket*  client = socket->Accept(&address);
			if (!client)
			{
				LIBNETWORK_LOG_T_F(LS_ERROR) << "accept failed !!!";
				return;
			}
			LIBNETWORK_LOG_T_F(LS_INFO) << "tcp new client accept :  " << address.ToString();
			std::unique_ptr<libnetwork::Connection>  tcp_session = std::make_unique<libnetwork::Connection>(client );
			//http_session->RegisterDecodeCompleteCallback(callback_);
			tcp_session->SignalOnRecv.connect(this, &TcpServer::OnSessionRecv);
			tcp_session->SignalOnClose.connect(this, &TcpServer::OnSessionClose);
			if (contexts_[kUserContext])
			{
				tcp_session->SetContext(kUserContext, contexts_[kUserContext]);
			}
			tcp_sessions_.emplace(std::make_pair(client, std::move(tcp_session)));
			auto iter = tcp_sessions_.find(client);
			if (iter == tcp_sessions_.end())
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "tcp session not find failed !!! socket: " << client->GetLocalAddress().ToString() << ", remote:" << client->GetRemoteAddress().ToString();
				return;
			}
			SignalOnNewConnection(iter->second.get());
		}
		void TcpServer::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";

		}
	}


}


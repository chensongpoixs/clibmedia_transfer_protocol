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
#include "libmedia_transfer_protocol/libhttp/http_server.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/libhttp/http_context.h"
#include "libmedia_transfer_protocol/libnetwork/connection.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{



		HttpServer::HttpServer()
			: tcp_server_(new libnetwork::TcpServer())
		{
			InitSocketSignals();
		}

		HttpServer::~HttpServer()
		{
			if (tcp_server_)
			{
				tcp_server_->SignalOnNewConnection.disconnect(this);
				tcp_server_->SignalOnRecv.disconnect(this);
				tcp_server_->SignalOnSent.disconnect(this);
				tcp_server_->SignalOnDestory.disconnect(this);
				tcp_server_.reset();
			}
			
		}

		bool HttpServer::Startup(const std::string &ip, uint16_t port)
		{
			return tcp_server_->network_thread()->Invoke<bool>(RTC_FROM_HERE, [this, ip, port]() {
			
				return tcp_server_->Startup(ip, port);
			});
			
			 
		}
		void HttpServer::InitSocketSignals()
		{
			//control_socket_->SignalCloseEvent.connect(this, &HttpServer::OnClose);
			//control_socket_->SignalConnectEvent.connect(this, &HttpServer::OnConnect);
			//control_socket_->SignalReadEvent.connect(this, &HttpServer::OnRead);
			//control_socket_->SignalWriteEvent.connect(this, &HttpServer::OnWrite);
			tcp_server_->SignalOnNewConnection.connect(this, &HttpServer::OnNewConnection);
			tcp_server_->SignalOnRecv.connect(this, &HttpServer::OnRecv);
			tcp_server_->SignalOnSent.connect(this, &HttpServer::OnSent);
			tcp_server_->SignalOnDestory.connect(this, &HttpServer::OnDestory);
		}

		void HttpServer::OnNewConnection(libnetwork::Connection* conn)
		{
			SignalOnNewConnection(conn);
			std::shared_ptr<HttpContext> shake = std::make_shared<HttpContext>(conn);
			
			{
				shake->SignalOnSent.connect(this, &HttpServer::OnSent);
				shake->SignalOnSentNextChunk.connect(this, &HttpServer::OnSentNextChunk);
				shake->SignalOnRequest.connect(this, &HttpServer::OnRequest);
			}
			conn->SetContext(libnetwork::kHttpContext, shake);
		}
		void HttpServer::OnDestory(libnetwork::Connection* conn)
		{
			SignalOnDestory(conn);
			std::shared_ptr<HttpContext> shake = conn->GetContext<HttpContext>(libnetwork::kHttpContext);
			if (shake)
			{
				shake->SignalOnSent.disconnect(this );
				shake->SignalOnSentNextChunk.disconnect(this );
				shake->SignalOnRequest.disconnect(this );
			}
			conn->ClearContext(libnetwork::kHttpContext);
		}
		void HttpServer::OnRecv(libnetwork::Connection* conn, const rtc::CopyOnWriteBuffer& data)
		{
			//SignalOnRecv(conn, data);
			std::shared_ptr<HttpContext> s = conn->GetContext<HttpContext>(libnetwork::kHttpContext);
			if (s)
			{
				MsgBuffer buffer;
				buffer.Append((const char *)data.data(), data.size());
				int32_t ret = s->Parse(buffer);
				if (ret == -1)
				{
					conn->Close();
				}
			}
		}
		void HttpServer::OnSent(libnetwork::Connection* conn)
		{
			std::shared_ptr<HttpContext> shake = conn->GetContext<HttpContext>(libnetwork::kHttpContext);
			if (shake)
			{
				shake->WriteComplete( conn);
			}
			//FlvContextPtr flv = conn->GetContext<FlvContext>(kFlvContext);
			//if (flv)
			//{
			//	flv->WriteComplete(std::dynamic_pointer_cast<TcpConnection>(conn));
			//}
			//SignalOnSent(conn);
		}
		void HttpServer::OnSentNextChunk(libnetwork::Connection *conn)
		{
			SignalOnSentNextChunk(conn);
		}
		void HttpServer::OnRequest(libnetwork::Connection *conn, const  std::shared_ptr<HttpRequest> http_request, const std::shared_ptr<Packet> packet)
		{
			SignalOnRequest(conn, http_request, packet);
		}
	}

	
}


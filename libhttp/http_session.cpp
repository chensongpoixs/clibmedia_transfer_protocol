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
#include "libmedia_transfer_protocol/libhttp/http_session.h" 
#include "rtc_base/logging.h"
namespace libmedia_transfer_protocol
{
	namespace libhttp
	{
#if 0
		HttpSession::HttpSession(TcpSession * tcp_session, rtc::Thread* network_thread)
			: tcp_session_(tcp_session)
			, network_thread_(network_thread) 
		{
			InitSocketSignals(); 
		}
		HttpSession::~HttpSession()
		{
		}
		void HttpSession::Close()
		{
			tcp_session_->Close();
		}
		void HttpSession::Send(uint8_t * data, int32_t size)
		{
			tcp_session_->Send(data, size);
		}
		void HttpSession::InitSocketSignals()
		{
			socket_->SignalCloseEvent.connect(this, &HttpSession::OnClose);
			socket_->SignalConnectEvent.connect(this, &HttpSession::OnConnect);
			socket_->SignalReadEvent.connect(this, &HttpSession::OnRead);
			socket_->SignalWriteEvent.connect(this, &HttpSession::OnWrite);
		}
		void HttpSession::OnConnect(rtc::Socket* socket)
		{
			LIBHTTP_LOG_F(LS_INFO) << "";
		}
		void HttpSession::OnClose(rtc::Socket* socket, int ret)
		{
			LIBHTTP_LOG_F(LS_INFO) << "";
		}
		void HttpSession::OnRead(rtc::Socket* socket)
		{
			LIBHTTP_LOG_F(LS_INFO) << "";

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
				if (read_bytes >= (buffer.capacity() ))
				{
					break;
				}
			} while (true);
			 
		}
		void HttpSession::OnWrite(rtc::Socket* socket)
		{
			LIBHTTP_LOG_F(LS_INFO) << "";
		}
#endif // 
	}
}
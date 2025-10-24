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
#include "libmedia_transfer_protocol/libnetwork/tcp_session.h"
#include "rtc_base/internal/default_socket_server.h"
#include "libice/stun.h"
#include "rtc_base/third_party/base64/base64.h"
#include "rtc_base/message_digest.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_util.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/common_header.h"
#include "rtc_base/string_encode.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
#include "rtc_base/logging.h"
namespace libmedia_transfer_protocol
{
	namespace libnetwork
	{
		TcpSession::TcpSession(rtc::Socket * socket, rtc::Thread * network_thread)
			: socket_(socket)
			, network_thread_(network_thread)
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(false)
		{
			LIBRTC_LOG_T_F(LS_INFO) << "";
			InitSocketSignals();
			 
		}
		TcpSession::~TcpSession()
		{
			LIBRTC_LOG_T_F(LS_INFO) << "";
			socket_->SignalCloseEvent.disconnect(this);
			socket_->SignalConnectEvent.disconnect(this);
			socket_->SignalReadEvent.disconnect(this);
			socket_->SignalWriteEvent.disconnect(this);
		}
		void TcpSession::Close()
		{
			available_write = false;
			socket_->Close();
		}
		void TcpSession::Send(uint8_t * data, int32_t size)
		{
			socket_->Send(data, size);
		}
		void TcpSession::InitSocketSignals()
		{
			socket_->SignalCloseEvent.connect(this, &TcpSession::OnClose);
			socket_->SignalConnectEvent.connect(this, &TcpSession::OnConnect);
			socket_->SignalReadEvent.connect(this, &TcpSession::OnRead);
			socket_->SignalWriteEvent.connect(this, &TcpSession::OnWrite);
		}
		void TcpSession::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		void TcpSession::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			SignalOnClose(this);
		}
		void TcpSession::OnRead(rtc::Socket* socket)
		{
			//LIBTCP_LOG_T_F(LS_INFO) << "";

			rtc::Buffer buffer(1024 * 1024 * 8);
			buffer.SetSize(0);
			 
			 
			do {
				int bytes = socket->Recv(buffer.begin() + buffer.size(), buffer.capacity() - buffer.size(), nullptr);
				if (bytes <= 0)
					break;
				//read_bytes += buffer;
				buffer.SetSize(  buffer.size() + bytes);
				if (buffer.size() >= (buffer.capacity()))
				{
					break;
				}
			} while (true);
			
			SignalOnRecv(this, rtc::CopyOnWriteBuffer (buffer));

		}
		void TcpSession::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = true;
		}
		void TcpSession::SetContext(int type, const std::shared_ptr<void> &context)
		{
			contexts_[type] = context;
		}
		void TcpSession::SetContext(int type, std::shared_ptr<void> &&context)
		{
			contexts_[type] = std::move(context);
		}
		void TcpSession::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		void TcpSession::ClearContext()
		{
			contexts_.clear();
		}
	}
}
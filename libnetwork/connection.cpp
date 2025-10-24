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

 
#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "libmedia_transfer_protocol/libnetwork/connection.h"

namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		Connection::Connection(rtc::AsyncPacketSocket * session, const rtc::SocketAddress& addr)
			: udp_session_(session) 
			, socket_(nullptr)
			, remote_address_(addr)
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(false)
			, protocol_type_(ProtocolType::ProtocolUdp)
		{
			LIBRTC_LOG_T_F(LS_INFO) << "";
			InitSocketSignals();
		}
		Connection::Connection(rtc::Socket * session)
			: udp_session_(nullptr) 
			, socket_(session)
			, remote_address_(session->GetRemoteAddress())
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(false)
			, protocol_type_(ProtocolType::ProtocolTcp)
		{
			LIBRTC_LOG_T_F(LS_INFO) << "";
			InitSocketSignals();
		}
		 
		Connection::~Connection()
		{
			LIBRTC_LOG_T_F(LS_INFO) << "";
			if (socket_)
			{
				socket_->SignalCloseEvent.disconnect(this);
				socket_->SignalConnectEvent.disconnect(this);
				socket_->SignalReadEvent.disconnect(this);
				socket_->SignalWriteEvent.disconnect(this);
			}
		}
		void Connection::Close()
		{
			available_write = false;
			if (socket_)
			{
				socket_->Close();
			}
			
		}
		void Connection::Send(uint8_t * data, int32_t size)
		{
			if (protocol_type_ == ProtocolType::ProtocolUdp)
			{
				udp_session_->SendTo(data, size, remote_address_, rtc::PacketOptions());
			}
			else //if ()
			{
				socket_->Send(data, size);
			}
		}
		void Connection::InitSocketSignals()
		{
			if (socket_)
			{
				socket_->SignalCloseEvent.connect(this, &Connection::OnClose);
				socket_->SignalConnectEvent.connect(this, &Connection::OnConnect);
				socket_->SignalReadEvent.connect(this, &Connection::OnRead);
				socket_->SignalWriteEvent.connect(this, &Connection::OnWrite);
			}
			
		}
		void Connection::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		void Connection::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			SignalOnClose(this);
		}
		void Connection::OnRead(rtc::Socket* socket)
		{
			//LIBTCP_LOG_T_F(LS_INFO) << "";

			rtc::Buffer buffer(1024 * 1024 * 8);
			buffer.SetSize(0);


			do {
				int bytes = socket->Recv(buffer.begin() + buffer.size(), buffer.capacity() - buffer.size(), nullptr);
				if (bytes <= 0)
					break;
				//read_bytes += buffer;
				buffer.SetSize(buffer.size() + bytes);
				if (buffer.size() >= (buffer.capacity()))
				{
					break;
				}
			} while (true);

			SignalOnRecv(this, rtc::CopyOnWriteBuffer(buffer));

		}
		void Connection::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = true;
		}
		void Connection::SetContext(int type, const std::shared_ptr<void> &context)
		{
			contexts_[type] = context;
		}
		void Connection::SetContext(int type, std::shared_ptr<void> &&context)
		{
			contexts_[type] = std::move(context);
		}
		void Connection::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		void Connection::ClearContext()
		{
			contexts_.clear();
		}
	}

}
 
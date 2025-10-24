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


#ifndef _C_LIBHTTP_CONNECTION_H_
#define _C_LIBHTTP_CONNECTION_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
#include <unordered_map>



namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		enum
		{
			kNormalContext = 0,
			kRtmpContext,
			kHttpContext,
			kUserContext,
			kFlvContext,
			kRtcContext,
			kGb28181Context,
		};


		enum class  ProtocolType{
			ProtocolUdp= 1,
			ProtocolTcp
		};
		typedef    UdpServer    UdpSession;
		class Connection : public   sigslot::has_slots<>
		{
		public:
			//explicit Connection();
			 Connection(rtc::AsyncPacketSocket * session, const rtc::SocketAddress& addr);
			  Connection(rtc::Socket * session);
			virtual ~Connection();
		public:

			void  Close();

			void Send(uint8_t *data, int32_t  size);


			rtc::Socket*   GetSocket() const { return socket_; }


			sigslot::signal1<Connection*> SignalOnClose;
			sigslot::signal2<Connection*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			sigslot::signal1<Connection*> SignalOnSent;
		public:

			
		public:
		private:
			
		public:
			 
			 

			void SetContext(int type, const std::shared_ptr<void> &context);
			void SetContext(int type, std::shared_ptr<void> &&context);
			template <typename T> std::shared_ptr<T> GetContext(int type) const
			{
				auto iter = contexts_.find(type);
				if (iter != contexts_.end())
				{
					return std::static_pointer_cast<T>(iter->second);
				}
				return std::shared_ptr<T>();
			}
			void ClearContext(int type);
			void ClearContext();

		private:
			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
		private:
			//UdpSession *        udp_session_;
			rtc::AsyncPacketSocket  * udp_session_;
			//TcpSession*        tcp_session_;
			rtc::Socket*  socket_;
			rtc::SocketAddress  remote_address_;
			rtc::Buffer  recv_buffer_;
			int32_t  recv_buffer_size_ = 0;
			std::atomic_bool         available_write;
			ProtocolType        protocol_type_ = ProtocolType::ProtocolUdp;

			std::unordered_map<uint32_t, std::shared_ptr<void>>     contexts_;
		};
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_
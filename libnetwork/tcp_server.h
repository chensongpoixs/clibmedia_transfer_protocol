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

GB28181使用RTP传输音视频，有两种方式：UDP、TCP。UDP和RTSP中的没有区别，但是TCP有区别。

		目前RTSP有两个版本1.0和2.0，1.0定义在RFC2326中，2.0定义在RFC7826。2.0是2016年由IETF发布的RTSP新标准，不过现在基本使用的都是RTSP1.0，就算有使用2.0的，也会兼容1.0。
		而GB28181则使用RFC4571中定义的RTP，这里面RTP over TCP方式和以往的不同。

		RFC2326中RTP over TCP的数据包是这样的：

| magic number | channel number | data length | data  |magic number -

magic number：   RTP数据标识符，"$" 一个字节
channel number： 信道数字 - 1个字节，用来指示信道
data length ：   数据长度 - 2个字节，用来指示插入数据长度
data ：          数据 - ，比如说RTP包，总长度与上面的数据长度相同
		而RFC4571中的RTP over TCP的数据包确是这样的：

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	---------------------------------------------------------------
   |             LENGTH            |  RTP or RTCP packet ...       |
	---------------------------------------------------------------
		RFC2326中用channel number标识消息类型，因为RTSP中信令和和音视频都是通过同一个TCP通道传输，所以必须通过channel number区分。而GB28181中信令和媒体数据是不同的传输通道，所以不用去区分。

		RFC4571标准格式：长度(2字节) + RTP头 + 数据

		RFC2326标准格式：$(1字节) + 通道号(1字节) + 长度(2字节) + RTP头 + 数据

 ******************************************************************************/

#ifndef _C_LIBHTTP_TCP_SERVER_H_
#define _C_LIBHTTP_TCP_SERVER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h" 
#include "rtc_base/thread.h"
#include "rtc_base/physical_socket_server.h"
#include "libp2p_peerconnection/connection_context.h"
//#include "libmedia_transfer_protocol/libnetwork/tcp_session.h"
//#include "libmedia_transfer_protocol/libnetwork/connection.h"
namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		class Connection;
		class TcpServer : public sigslot::has_slots<>  //: public   TcpHandler
		{
		public:
			explicit TcpServer();
			virtual ~TcpServer();

		public:

			bool Startup(const std::string &ip, uint16_t port); 

			void CloseSession(Connection *conn);
			void Close(rtc::Socket *socket);
		public: 
			sigslot::signal1<Connection*> SignalOnNewConnection;
			sigslot::signal2<Connection*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			sigslot::signal1<Connection*> SignalOnSent;
			sigslot::signal1<Connection*> SignalOnDestory;
			

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
		public:
			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }


	
		public:
			/// session
			void OnSessionRecv(Connection*  conn, const rtc::CopyOnWriteBuffer & data);
			void OnSessionClose(Connection*  conn);
		public:
			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);

		private:
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;

			rtc::SocketAddress server_address_;

			std::unique_ptr<rtc::Socket> control_socket_;
			rtc::AsyncResolver* resolver_;


			std::map<rtc::Socket*, std::unique_ptr<libnetwork::Connection>>						tcp_sessions_;
			std::unordered_map<int, std::shared_ptr<void>> contexts_;

		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
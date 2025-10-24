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


#ifndef _C_LIBHTTP_TCP_SESSION_H_
#define _C_LIBHTTP_TCP_SESSION_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		
#if 0
		class TcpSession : public   sigslot::has_slots<>
		{
		public:

			explicit TcpSession(rtc::Socket* socket );

			virtual ~TcpSession();
		public:
			 
			void  Close();

			void Send(uint8_t *data, int32_t  size);


			rtc::Socket*   GetSocket() const { return socket_; }

			
			sigslot::signal1<TcpSession*> SignalOnClose;  
			sigslot::signal2<TcpSession*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			sigslot::signal1<TcpSession*> SignalOnSent;
		public:

			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
		public:
		private:
			rtc::Socket*  socket_; 
			rtc::Buffer  recv_buffer_;
			int32_t  recv_buffer_size_ = 0; 
			std::atomic_bool         available_write  ;
		};
#endif //
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_
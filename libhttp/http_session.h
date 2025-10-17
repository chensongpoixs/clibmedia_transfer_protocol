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


#ifndef _C_LIBHTTP_SESSION_H_
#define _C_LIBHTTP_SESSION_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 ////////////////////
#include "libp2p_peerconnection/connection_context.h"
#include "libmedia_transfer_protocol/libhttp/tcp_server.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{

#if 0
		class HttpSession : public   sigslot::has_slots<>
		{
		public:

			explicit HttpSession(TcpSession * tcp_session, rtc::Thread* network_thread);

			virtual ~HttpSession();
		public:
			//void RegisterDecodeCompleteCallback(libcross_platform_collection_render::cvideo_renderer * callback)
			//{
			//	callback_ = callback;
			//
			//	//h264_decoder_.RegisterDecodeCompleteCallback(callback);
			//}
			void  Close();

			void Send(uint8_t *data, int32_t  size);
		public:

			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
		public:
			TcpSession*  tcp_session_;
			 rtc::Thread*  network_thread_; 
		};
#endif //
	}

}


#endif // _C_LIBGB28181_SESSION_H_
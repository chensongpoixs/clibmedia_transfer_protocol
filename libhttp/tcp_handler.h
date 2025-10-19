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

#ifndef _C_LIBHTTP_TCP_HANDLER_H_
#define _C_LIBHTTP_TCP_HANDLER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h" 
#include "rtc_base/thread.h"
#include "rtc_base/physical_socket_server.h"
#include "libp2p_peerconnection/connection_context.h"
#include "libmedia_transfer_protocol/libhttp/tcp_session.h"
#include "libmedia_transfer_protocol/libhttp/http_request.h"
#include "libmedia_transfer_protocol/libhttp/packet.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		class TcpHandler : public sigslot::has_slots<>
		{
		public:
			sigslot::signal1<TcpSession*> SignalOnNewConnection;
			sigslot::signal1<TcpSession*> SignalOnDestory;
			sigslot::signal2<TcpSession*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			//sigslot::signal1<TcpSession*> SignalOnSent;

			//////
			sigslot::signal1<  TcpSession *> SignalOnSent;
			sigslot::signal1<  TcpSession *> SignalOnSentNextChunk;
			sigslot::signal3<  TcpSession *, const  std::shared_ptr<HttpRequest>, const std::shared_ptr<Packet>> SignalOnRequest;
		};
	}
}

#endif // _C_LIBHTTP_TCP_HANDLER_H_
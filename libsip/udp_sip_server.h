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
				   date:  2025-10-11



 ******************************************************************************/


#ifndef _C_UDP_SIP_SERVER_H_
#define _C_UDP_SIP_SERVER_H_

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "libp2p_peerconnection/connection_context.h"
namespace libmedia_transfer_protocol {


	namespace libsip
	{
		class UdpSipServer : public   sigslot::has_slots<>
		{
		public:
			UdpSipServer(rtc::Thread* network_thread, rtc::Thread* work_thread);
			~UdpSipServer();
		public:

			bool Start(const char *ip, uint16_t port);
		public:

			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
		public:
			rtc::Thread *      network_thread_;
			rtc::Thread *      work_thread_;
			rtc::SocketAddress server_address_;

			std::unique_ptr<rtc::Socket> control_socket_;
		};
	}
}

#endif // _C_UDP_SERVER_H_
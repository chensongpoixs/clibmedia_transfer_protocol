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


#ifndef _C_SIP_SERVER_H_
#define _C_SIP_SERVER_H_

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "libp2p_peerconnection/connection_context.h"
#include "libmedia_transfer_protocol/libsip/udp_sip_server.h"
//#include "libmedia_transfer_protocol/libsip/sip_server.h"
#include "libmedia_transfer_protocol/libsip/sip_message.h"
namespace libmedia_transfer_protocol {


	namespace libsip
	{
		class SipServer
		{
		public:
			SipServer();
			~SipServer();

		public:
			bool Start();

		public:
			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }

		public:
			// handler method name 
			void handler_register(const SipMessage& message, const rtc::SocketAddress & address);

			void handler_message(const SipMessage& message, const rtc::SocketAddress & address);
		public:

		private:
			/*
				sip服务id：41010500002000000001
				sip服务域：3402000000
				sip服务ip:192.168.1.2
				sip服务端口：15060
				sip用户名:41010500002000000003
				sip用户认证id:41010500002000000003
			*/
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;
			std::string            server_id_ = "41010500002000000001";
			std::string            server_once_ = "3402000000";
			std::string            server_password_ = "12345678";
			std::string            server_ip_ = "192.168.1.2";
			uint16_t			   server_port_ = 15060;

			std::unique_ptr<UdpSipServer>    udp_sip_server_;
		};
	}

}


#endif // libmedia_transfer_protocol
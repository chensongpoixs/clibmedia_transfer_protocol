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
#include "libmedia_transfer_protocol/libsip/udp_sip_server.h"
#include "libmedia_transfer_protocol/libsip/sip_server.h"
#include "libmedia_transfer_protocol/libsip/sip_message.h"
namespace libmedia_transfer_protocol
{
	namespace libsip
	{
		SipServer::SipServer()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
		{
		}
		SipServer::~SipServer()
		{
		}
		bool SipServer::Start()
		{
			udp_sip_server_ = std::make_unique<UdpSipServer>(context_->network_thread(), context_->worker_thread());
			udp_sip_server_->Start(server_ip_.c_str(), server_port_);
			return true;
		}
		void SipServer::handler_register(const SipMessage & message, const rtc::SocketAddress & address)
		{

		}
		void SipServer::handler_message(const SipMessage & message, const rtc::SocketAddress & address)
		{
		}
	}
}
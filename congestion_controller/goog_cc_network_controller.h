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
				   date:  2025-09-28



 ******************************************************************************/


#ifndef _C_GOOG_CC_NETWORK_CONTROLLER_H_
#define _C_GOOG_CC_NETWORK_CONTROLLER_H_
#include "libmedia_transfer_protocol/media_config.h"
#include "libmedia_transfer_protocol/congestion_controller/delay_based_bwe.h"
#include "libmedia_transfer_protocol/network_controller.h"
namespace libmtp
{
	class GoogCcNetworkController : public NetworkControllerInterface
	{
	public:
		GoogCcNetworkController();
		virtual ~GoogCcNetworkController() override;
		virtual libice::NetworkControlUpdate OnTransportPacketsFeedback(
			const libice::TransportPacketsFeedback& msg) override;

		virtual  libice::NetworkControlUpdate OnRttUpdate(int64_t rtt_ms) override;
	private:
		std::unique_ptr<DelayBasedBwe>  delay_based_bwe_;
	};
}

#endif // _C_GOOG_CC_NETWORK_CONTROLLER_H_
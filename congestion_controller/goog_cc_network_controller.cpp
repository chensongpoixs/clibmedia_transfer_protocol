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

#include "libmedia_transfer_protocol/congestion_controller/goog_cc_network_controller.h"


namespace libmtp
{
	GoogCcNetworkController::GoogCcNetworkController()
		: delay_based_bwe_( std::make_unique< DelayBasedBwe>())
	{

	}
	GoogCcNetworkController:: ~GoogCcNetworkController()
	{

	}
	libice::NetworkControlUpdate GoogCcNetworkController::OnTransportPacketsFeedback(
		const libice::TransportPacketsFeedback & report)
	{
		if (report.packet_feedbacks.empty())
		{
			return libice::NetworkControlUpdate();
		}

		DelayBasedBwe::Result result = delay_based_bwe_->IncomingPacketFeedbackVector(report, true);
		return libice::NetworkControlUpdate();
	}
}
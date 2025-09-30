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


#ifndef _C__NETWORK_CONTROLLER_H_
#define _C__NETWORK_CONTROLLER_H_
#include "libmedia_transfer_protocol/media_config.h"
#include "libice/network_types.h"

namespace libmtp
{

	// 接口类
	class NetworkControllerInterface
	{
	public:
		virtual ~NetworkControllerInterface() {}
		// transport-cc接受端反馈信息接口
		virtual libice::NetworkControlUpdate OnTransportPacketsFeedback(const libice::TransportPacketsFeedback&) = 0;
	
		virtual  libice::NetworkControlUpdate OnRttUpdate(int64_t rtt_ms, webrtc::Timestamp at_time) = 0;
		// Called when network availabilty changes.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnNetworkAvailability(
		//	NetworkAvailability) = 0;
		//// Called when the receiving or sending endpoint changes address.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnNetworkRouteChange(
		//	NetworkRouteChange) = 0;
		//// Called periodically with a periodicy as specified by
		//// NetworkControllerFactoryInterface::GetProcessInterval.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnProcessInterval(
		//	ProcessInterval) = 0;
		//// Called when remotely calculated bitrate is received.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnRemoteBitrateReport(
		//	RemoteBitrateReport) = 0;
		//// Called round trip time has been calculated by protocol specific mechanisms.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnRoundTripTimeUpdate(
		//	RoundTripTimeUpdate) = 0;
		//// Called when a packet is sent on the network.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnSentPacket(
		//	SentPacket) = 0;
		//// Called when a packet is received from the remote client.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnReceivedPacket(
		//	ReceivedPacket) = 0;
		//// Called when the stream specific configuration has been updated.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnStreamsConfig(
		//	StreamsConfig) = 0;
		//// Called when target transfer rate constraints has been changed.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnTargetRateConstraints(
		//	TargetRateConstraints) = 0;
		//// Called when a protocol specific calculation of packet loss has been made.
	  virtual libice::NetworkControlUpdate OnTransportLossReport(
		  libice::TransportLossReport) = 0;
		//// Called with per packet feedback regarding receive time.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnTransportPacketsFeedback(
		//	TransportPacketsFeedback) = 0;
		//// Called with network state estimate updates.
		//ABSL_MUST_USE_RESULT virtual NetworkControlUpdate OnNetworkStateEstimate(
		//	NetworkStateEstimate) = 0;
	};
}

#endif // _C__NETWORK_CONTROLLER_H_
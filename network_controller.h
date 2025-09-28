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
	};
}

#endif // _C__NETWORK_CONTROLLER_H_
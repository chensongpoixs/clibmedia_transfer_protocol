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

 GB28181中SIP URL格式为sip:<SIP监控域ID>@<ip>:<port>，SIP监控域ID有20位。


 ******************************************************************************/


#ifndef _C_SIP_MEESSAGE_H_
#define _C_SIP_MEESSAGE_H_

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "libp2p_peerconnection/connection_context.h"
#include <map>
#include <string>
namespace libmedia_transfer_protocol {


	namespace libsip
	{
		class SipMessage
		{
		public:
			SipMessage();
			~SipMessage();

			bool Parse(const char * data, int32_t size);
		public:

			std::string   method_name_;
			std::map<std::string, std::string>    data_map_;
			std::vector<std::string>  lines_;
		};
	}
}


#endif // 
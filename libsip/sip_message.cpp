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
#include "libmedia_transfer_protocol/libsip/sip_message.h"
#include "rtc_base/string_utils.h"

namespace libmedia_transfer_protocol
{
	namespace libsip
	{
		SipMessage::SipMessage()
		{
		}
		SipMessage::~SipMessage()
		{
		}
		bool SipMessage::Parse(const char * data, int32_t size)
		{


			
			rtc::split(std::string(data, size), '\n', &lines_);
			if (lines_.size() < 1)
			{
				RTC_LOG(LS_WARNING) << "parse line tail samll size: " << lines_.size() << ", data:" << std::string(data, size);
				return false;
			}
			// method name 
			auto  start_index = lines_[0].find_first_of(' ');
			//截取 method name
			method_name_ = lines_[0].substr(0, start_index);

			return false;
		}
	}
}
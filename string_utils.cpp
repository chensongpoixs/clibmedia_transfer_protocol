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
				   date:  2025-10-14



 ******************************************************************************/
#include "libmedia_transfer_protocol/string_utils.h"

namespace libmedia_transfer_protocol
{
	bool  StringUtils::StartsWith(const std::string& s, const std::string & sub)
	{
		if (sub.empty())
		{
			return true;
		}
		if (s.empty())
		{
			return false;
		}
		auto len = s.size();
		auto slen = sub.size();
		if (len < slen)
		{
			return false;
		}
		return s.compare(0, slen, sub) == 0;
		 
	  }
	bool StringUtils:: EndsWith(const std::string &s, const std::string& sub)
	{

		if (sub.empty())
		{
			return true;
		}
		if (s.empty())
		{
			return false;
		}
		auto len = s.size();
		auto slen = sub.size();
		if (len < slen)
		{
			return false;
		}
		return s.compare(len - slen, slen, sub) == 0;
	  }
}
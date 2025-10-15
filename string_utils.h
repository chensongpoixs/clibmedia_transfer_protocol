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


#ifndef _C_STRING_UTILS_H_
#define _C_STRING_UTILS_H_

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"

namespace libmedia_transfer_protocol {



	class StringUtils
	{
	public:

		static bool  StartsWith(const std::string& s, const std::string & sub);
		static bool  EndsWith(const std::string &s, const std::string& sub);

	};
}

#endif // _C_STRING_UTILS_H_
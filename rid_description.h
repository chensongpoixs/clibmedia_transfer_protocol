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
				   date:  2025-09-21



 ******************************************************************************/

#ifndef _C_MEDIA_BASE_RID_DESCRIPTION_H_
#define _C_MEDIA_BASE_RID_DESCRIPTION_H_

#include <map>
#include <string>
#include <vector>

namespace libmedia_transfer_protocol {

enum class RidDirection { kSend, kReceive };
 
struct RidDescription   {
   
	RidDescription();
	RidDescription(const std::string& rid, RidDirection direction);
	RidDescription(const RidDescription& other);
	~RidDescription();
	RidDescription& operator=(const RidDescription& other);

	// This is currently required for unit tests of StreamParams which contains
	// RidDescription objects and checks for equality using operator==.
	bool operator==(const RidDescription& other) const;
	bool operator!=(const RidDescription& other) const {
		return !(*this == other);
	}
  std::string rid;
   
  RidDirection direction;
   
  std::vector<int> payload_types;

 
  std::map<std::string, std::string> restrictions;
};

}  // namespace cricket

#endif  // MEDIA_BASE_RID_DESCRIPTION_H_

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
				   date:  2025-10-08


 ******************************************************************************/

 
#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "libmedia_transfer_protocol/libhttp/http_type.h"
#include <string>
#include <unordered_map>
#include <iostream>
 //#include <ctype.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <cassert>
#include "libmedia_transfer_protocol/libhttp/packet.h"

#include <algorithm>
#include "libmedia_transfer_protocol/libhttp/packet.h"

#include <assert.h>

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		std::shared_ptr<Packet> Packet::NewPacket(int32_t size)
		{
			auto block_size = size + sizeof(Packet);
			Packet * packet = (Packet*)new char[block_size];
			memset((void*)packet, 0x00, block_size);
			packet->index_ = -1;
			packet->type_ = kPacketTypeUnknowed;
			packet->capacity_ = size;
			packet->ext_.reset();

			return std::shared_ptr<Packet>(packet, [](Packet *p) {
				delete[](char*)p;
			});
		}
	}
}
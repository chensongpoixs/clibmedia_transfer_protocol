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
				   date:  2025-10-22



 ******************************************************************************/


#ifndef _C_RTC_ERRORS_H_
#define _C_RTC_ERRORS_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <map>
#include <stdexcept>

namespace libmedia_transfer_protocol {
	namespace librtc {

		class RtcError :public std::runtime_error
		{
		public:
			explicit RtcError(const char* description) : std::runtime_error(description)
			{
			}

		public:
			static const size_t bufferSize{ 2000 };
			thread_local static char buffer[];
		};

		// clang-format off
#define MS_THROW_ERROR(desc, ...) \
	do \
	{ \
		std::snprintf(libmedia_transfer_protocol::librtc::RtcError::buffer, libmedia_transfer_protocol::librtc::RtcError::bufferSize, desc, ##__VA_ARGS__); \
		throw libmedia_transfer_protocol::librtc::RtcError(libmedia_transfer_protocol::librtc::RtcError::buffer); \
	} while (false)
	}
}


#endif 
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
#include "libmtp/ccodec.h"
#include <string>
#include <map>
#include <vector>
#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "rtc_base/logging.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/strings/string_builder.h"
namespace libmtp
{
	std::string AudioCodec::ToString() const
	{
		char buf[256];
		rtc::SimpleStringBuilder sb(buf);
		sb << "AudioCodec[" << id << ":" << name << ":" << clockrate << ":" << bitrate
			<< ":" << channels << "]";
		return sb.str();
	}


	std::string VideoCodec::ToString() const
	{
		char buf[256];
		rtc::SimpleStringBuilder sb(buf);
		sb << "VideoCodec[" << id << ":" << name;
		if (packetization.has_value()) {
			sb << ":" << *packetization;
		}
		sb << "]";
		return sb.str();
	}

	VideoCodec::CodecType VideoCodec::GetCodecType() const
	{
		if (absl::EqualsIgnoreCase(name, kRedCodecName)) {
			return CODEC_RED;
		}
		if (absl::EqualsIgnoreCase(name, kUlpfecCodecName)) {
			return CODEC_ULPFEC;
		}
		if (absl::EqualsIgnoreCase(name, kFlexfecCodecName)) {
			return CODEC_FLEXFEC;
		}
		if (absl::EqualsIgnoreCase(name, kRtxCodecName)) {
			return CODEC_RTX;
		}

		return CODEC_VIDEO;
	}
}
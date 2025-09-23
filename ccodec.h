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

#ifndef _C_CODEC_H_
#define _C_CODEC_H_

#include <string>
#include <map>
#include <vector>
#include "libmtp/media_constants.h"
#include "absl/types/optional.h"
namespace libmtp
{
 
	struct FeedbackParam { 
 
		std::string id_   ;     // e.g. "nack", "ccm"
		std::string param_ = libmtp::kParamValueEmpty;  // e.g. "", "rpsi", "fir"
	};  
	struct   Codec {
		int id;
		std::string name;
		int clockrate;

		virtual  size_t  GetChannel() const   { return 0; }
		// Non key-value parameters such as the telephone-event "0‐15" are
		// represented using an empty string as key, i.e. {"": "0-15"}.
		// fmtp
		std::map<std::string, std::string> params;
		// rtcp-fb
		std::vector<FeedbackParam> feedback_params;
	};



	struct AudioCodec : public Codec {
		int bitrate;
		size_t channels;
		virtual  size_t  GetChannel() const override { return channels; }
		 
		std::string ToString() const; 
	};

	struct   VideoCodec : public Codec {
		absl::optional<std::string> packetization;

		 

		std::string ToString() const;

		

		enum CodecType {
			CODEC_VIDEO,
			CODEC_RED,
			CODEC_ULPFEC,
			CODEC_FLEXFEC,
			CODEC_RTX,
		}; 
		CodecType GetCodecType() const;
	};

}

#endif // _C_CODEC_H_
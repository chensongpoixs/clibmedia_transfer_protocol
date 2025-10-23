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
				   date:  2025-10-23



 ******************************************************************************/


#ifndef _C_MEDIA_TRENASFER_PROTOCOL_MUXER_H_
#define _C_MEDIA_TRENASFER_PROTOCOL_MUXER_H_
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_codec/audio_codec/opus_encoder.h"
#include "libmedia_codec/audio_codec/aac_decoder.h"
#include "libmedia_codec/audio_codec/adts_header.h"

namespace libmedia_transfer_protocol
{
	class Muxer
	{
	public:
		explicit Muxer(libmedia_codec::EncodeAudioObser   * obj);
		~Muxer();


		int32_t EncodeAudio(const rtc::CopyOnWriteBuffer & frame);

	public:
	private:
		libmedia_codec::EncodeAudioObser   *            encoder_audio_obj_;
		//libmedia_codec::AdtsHeader       adts_header_;
		std::shared_ptr<libmedia_codec::AacDecoder>     aac_decoder_;
		std::shared_ptr<libmedia_codec::OpusEncoder2>   opus_encoder_;
	};
}

#endif // _C_MEDIA_TRENASFER_PROTOCOL_MUXER_H_
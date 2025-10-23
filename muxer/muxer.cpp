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
#include "libmedia_transfer_protocol/muxer/muxer.h"
#include "libmedia_codec/audio_codec/adts_header.h"
#include  "rtc_base/logging.h"
#include "rtc_base/thread_annotations.h"
namespace libmedia_transfer_protocol
{
	namespace
	{
		const int ff_mpeg4audio_sample_rates[16] = {
		   96000, 88200, 64000, 48000, 44100, 32000,
		   24000, 22050, 16000, 12000, 11025, 8000, 7350
		};
	}
	Muxer::Muxer(libmedia_codec::EncodeAudioObser   * obj)
		: encoder_audio_obj_(obj)
	{
	}
	Muxer::~Muxer()
	{
		RTC_LOG_T_F(LS_INFO) << "";
		if (opus_encoder_)
		{
			opus_encoder_->Stop();
			opus_encoder_.reset();
		}
		if (aac_decoder_)
		{
			aac_decoder_.reset();
		}
	}
	static int get_one_ADTS_frame(unsigned char* buffer, size_t buf_size, unsigned char* data, size_t* data_size)
	{
		size_t size = 0;

		if (!buffer || !data || !data_size)
		{
			return -1;
		}

		while (1)
		{
			if (buf_size < 7)
			{
				return -1;
			}

			if ((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0))
			{
				size |= ((buffer[3] & 0x03) << 11);     //high 2 bit  
				size |= buffer[4] << 3;                //middle 8 bit  
				size |= ((buffer[5] & 0xe0) >> 5);        //low 3bit  
				break;
			}
			--buf_size;
			++buffer;
		}

		if (buf_size < size)
		{
			return -1;
		}

		memcpy(data, buffer, size);
		*data_size = size;

		return 0;
	}

	int32_t Muxer::EncodeAudio(const rtc::CopyOnWriteBuffer & frame)
	{
		//static unsigned char new_frame[1024 * 5 * 1024];
		//size_t   size = 1024 * 1024;
		libmedia_codec::T_AdtsHeader   aac_adts_header_info;
		if (!aac_decoder_)
		{
			//adts_header_.parse(frame.data(), frame.size());
			
			//if (get_one_ADTS_frame((unsigned char*)frame.data(),(size_t) frame.size(), new_frame, &size) < 0)
			//{
			//	return -1;
			//}
			int ret = libmedia_codec::getAdtsFrame((const uint8_t *)frame.data(), frame.size(), &aac_adts_header_info);
			if (ret != 0)
			{
				LIBMEIDA_CODEC_LOG_T_F(LS_WARNING)<<"get adts frame : " << ret;
			}
			RTC_ASSERT(frame.size() == aac_adts_header_info.aac_frame_length, " aac adts data size : %u != aac length: %u !", frame.size(), aac_adts_header_info.aac_frame_length);
			aac_decoder_ = std::make_shared<libmedia_codec::AacDecoder>();
			//aac_decoder_->Init(adts_header_.acc_adts_header_info_.sample_rate, adts_header_.acc_adts_header_info_.crc_absent);
			aac_decoder_->Init((unsigned char *)frame.data(), aac_adts_header_info.aac_frame_length );
			if (!opus_encoder_)
			{
				opus_encoder_ = std::make_shared<libmedia_codec::OpusEncoder2>();
				opus_encoder_->SetSendFrame(encoder_audio_obj_);
				opus_encoder_->SetChannel( aac_adts_header_info.channel_configuration );
				opus_encoder_->SetSample(ff_mpeg4audio_sample_rates[aac_adts_header_info.sampling_freq_index]/*aac_adts_header_info.sampling_freq_index*/);
				opus_encoder_->Start();
			}
			return 0;
		}
		

		//adts_header_.parse(frame.data(), frame.size());
		//if (get_one_ADTS_frame((unsigned char*)frame.data(), (size_t)frame.size(), new_frame, &size) < 0)
		//{
		//	return -1;
		//}
		int ret = libmedia_codec::getAdtsFrame((const uint8_t *)frame.data(), frame.size(), &aac_adts_header_info);
		if (ret != 0)
		{
			LIBMEIDA_CODEC_LOG_T_F(LS_WARNING) << "get adts frame : " << ret;
		}
		RTC_ASSERT(frame.size() == aac_adts_header_info.aac_frame_length, " aac adts data size : %u != aac length: %u !", frame.size(), aac_adts_header_info.aac_frame_length);

		rtc::Buffer  pcm  = aac_decoder_->Decode((unsigned char *)(frame.data()), aac_adts_header_info.aac_frame_length);
		if (pcm.size() <= 0)
		{
			return 0;
		}
#if 0
		static FILE * out_file_ptr = fopen("aac_ch_.pcm", "wb+");
		if (out_file_ptr)
		{
			fwrite(pcm.data(), pcm.size(), 1, out_file_ptr);
			fflush(out_file_ptr);
		}
#endif //


#if 1
		// samplerate:32000 channels:2
		std::shared_ptr<libmedia_codec::AudioFrame> pcm_frame = std::make_shared<libmedia_codec::AudioFrame>();
		pcm_frame->sample_rate_hz_ =   ff_mpeg4audio_sample_rates[aac_adts_header_info.sampling_freq_index];
		pcm_frame->num_channels_ =   aac_adts_header_info.channel_configuration;
		pcm_frame->samples_per_channel_ = 960;// 480;// pcm.size();
		//RTC_LOG(LS_INFO) << "sample_rate_hz:" << pcm_frame->sample_rate_hz_ << ", samples_per_channel_: " << pcm_frame->samples_per_channel_;
		memcpy((   uint8_t  *)(pcm_frame->mutable_data()), pcm.data(), pcm.size());
		opus_encoder_->OnNewMediaFrame(pcm_frame);
#endif 
		//return int32_t();
		return 0;
	}
}
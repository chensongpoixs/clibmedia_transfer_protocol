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
				   date:  2025-10-09



 ******************************************************************************/
#include "libmedia_transfer_protocol/libmpeg/mpeg_decoder.h"
#include "rtc_base/logging.h"
#include "libmedia_codec/encoded_frame.h"
#include "libmedia_codec/encoded_image.h"
#include "libmedia_transfer_protocol/video_receive_stream.h"
#include "libmedia_codec/audio_codec/audio_decoder.h"
extern "C" {
	//#include "lib"
	//#include "libavcodec/avcodec.h"
	//#include "libavutil/imgutils.h"
#include "libavcodec/adts_parser.h"
#include "libavcodec/adts_parser.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
}  // extern "C"


#include <stdint.h>
#if defined(_MSC_VER)
#include <stdlib.h>
#include <intrin.h>
#endif


namespace libmedia_transfer_protocol {
	namespace libmpeg
	{

		namespace {
			static  const uint32_t    kPsHeaderCode = 0x000001BA;
			static const uint32_t    kPsSystemHeaderCode = 0x000001BB;
			static const uint32_t     kPsPsmHeaderCode = 0x000001BC;

			static const uint32_t  kPsPesVideo = 0x000001E0;  // 视频
			static const uint32_t  kPsPesAudio = 0x000001C0;// 音频

#pragma pack (1)
			typedef union littel_endian_size_s {
				unsigned short int	length;
				unsigned char		m_byte[2];
			} littel_endian_size;

			typedef struct pack_start_code_s {
				unsigned char start_code[3];
				unsigned char stream_id[1];
			} pack_start_code;

			typedef struct program_stream_pack_header_s {
				pack_start_code PackStart;// 4
				unsigned char Buf[9];
				unsigned char stuffinglen;
			} program_stream_pack_header;

			typedef struct program_stream_map_s {
				pack_start_code PackStart;
				littel_endian_size PackLength;//we mast do exchange
			} program_stream_map;

			typedef struct program_stream_e_s {
				pack_start_code		PackStart;
				littel_endian_size	PackLength;//we mast do exchange
				char				PackInfo1[2];
				unsigned char		stuffing_length; // 填充数据
			} program_stream_e;
			//#pragma pop(1)




			//static inline int64_t ff_parse_pes_pts(const uint8_t *buf) {
			//	return (int64_t)(*buf & 0x0e) << 29 |
			//		(_rotr16(buf + 1, 8) >> 1) << 15 |
			//		_rotr16(buf + 3, 8) >> 1;
			//}
		}

		MpegDecoder::MpegDecoder()
			: h264_stream_((new uint8_t[1024 * 1024 * 8]))
			, stream_len_(0)
			, read_byte_(0)
			//, callback_(nullptr)
		{
		}

		MpegDecoder::~MpegDecoder()
		{
		}

		int MpegDecoder::parse(const uint8_t * data, int32_t size)
		{

			int32_t len = 0;;
			const uint8_t * ps = data;
			//static FILE *out_file_ptr = fopen("test_ps.mp4", "wb+");
			if (read_byte_ > 0)
			{
				if (size >= read_byte_)
				{
					// 
					//fwrite(ps, 1, read_byte_, out_file_ptr);
					//fflush(out_file_ptr);
					//read_byte_ = payload_size - avil_read_byte;
					memcpy(h264_stream_ + stream_len_, ps, read_byte_);
					stream_len_ += read_byte_;
					ps += read_byte_;
					len = read_byte_;
					read_byte_ = 0;
					
				}
				else if (size < read_byte_ )
				{
					//不足够一个包
					//fwrite(ps, 1, size, out_file_ptr);
					//fflush(out_file_ptr);
					memcpy(h264_stream_ + stream_len_, ps, size);
					stream_len_ += size;
					read_byte_ -= size;
					len = size;

				}
				else
				{
					LIBMPEG_LOG_T_F(LS_WARNING) << " size : " << size << ", len : " << len << ", read_byte_: " << read_byte_;;
				}
#if 0
				if (read_byte_ == 0)
				{
					if (callback_)
					{
						libmedia_codec::EncodedImage encode_image;
						encode_image.SetEncodedData(
							libmedia_codec::EncodedImageBuffer::Create(
								h264_stream_,
								stream_len_
							));
						callback_->onFrame(encode_image);
					}
					stream_len_ = 0;
				}
#endif // 
			 //RTC_LOG(LS_INFO)<< ", size : " << size << ", len : " << len << ", read_byte_: " << read_byte_;;
			}
			while  ((size - len) > 4)
			{
				pack_start_code *start_code =(pack_start_code*)(ps);

				//1、视频关键帧的封装 RTP + PS header + PS system header + PS system Map + PES header + h264 data

				//	2、视频非关键帧的封装 RTP + PS header + PES header + h264 data

				//	3、音频帧的封装: RTP + PES header + G711
				if (ps[0] == 0X00/*'\00'*/ &&
					ps[1] == 0X00/*'\00'*/ &&
					ps[2] == 0X01/*'\01'*/)
				{
					switch (start_code->stream_id[0])
					{
						case 0XBA:  // PS Header 
						{
 
							program_stream_pack_header* ps_header = (program_stream_pack_header*)ps;
							//找到填充数据的大小 后三位
							uint8_t    pack_stuffinglen = ps_header->stuffinglen & 0x0000007; // '\07';
							//
							// 中时钟需要取出来 SCR（系统时钟参考）
							// 去除 ps Header 包头的大小
							len  +=   /*sizeof(program_stream_pack_header)*/ (4 + 9 + 1 + pack_stuffinglen);
							ps += (4 + 9 + 1 + pack_stuffinglen);
							break;
						}
						case  0XBB/*'\bb'*/: // PS System Header
						{
							program_stream_map* PSMPack = (program_stream_map*)ps;
							if (size -len < (4 + 2))
							{
								LIBMPEG_LOG_T_F(LS_ERROR) << "parse PS    System Header packet tai samil (7)!!! len :" << len << " < 7 ";
								return -1;
							}

							littel_endian_size psm_length;
							psm_length.m_byte[0] = PSMPack->PackLength.m_byte[1];
							psm_length.m_byte[1] = PSMPack->PackLength.m_byte[0];

							len += psm_length.length + sizeof(program_stream_map);
							ps += psm_length.length + sizeof(program_stream_map);


							break;
						}
						case 0XBC/*'\bc'*/: // PSM  (Proprom Stream Map)
						{
							program_stream_map* PSMPack = (program_stream_map*)ps;
							if (size - len < (4 + 2))
							{
								LIBMPEG_LOG_T_F(LS_ERROR) << "parse PS   Proprom Stream Map  packet tai samil (7)!!! len :" << len << " < 7 ";
								return -1;
							}

							littel_endian_size psm_length;
							psm_length.m_byte[0] = PSMPack->PackLength.m_byte[1];
							psm_length.m_byte[1] = PSMPack->PackLength.m_byte[0];

							len += psm_length.length + sizeof(program_stream_map);
							ps += psm_length.length + sizeof(program_stream_map);
							break;
						}
						case 0XE0/*'\e0'*/:// PES（Packetized Elementary Stream）
						{
							/*
							
							 //  2 - PTS_DTS_flags()
								//  1 - ESCR_flag(0)
								//  1 - ES_rate_flag(0)
								//  1 - DSM_trick_mode_flag(0)
								//  1 - additional_copy_info_flag(0)
								//  1 - PES_CRC_flag(0)
								//  1 - PES_extension_flag()
 20 20 01 e0 7f ae 8c 20 03 ff ff f8 20 20 20 01 26 01 ad 13 80 8f 39 0b 51 6c c7 1e 8b 2f 7f 2d 20  
							*/


							// VIdeo
							program_stream_e* PSEPack = (program_stream_e*)ps;
							if (size - len < 9)
							{
								LIBMPEG_LOG_T_F(LS_ERROR) << "parse PES  video  packet tail small  len:" << len;
								return -1;
							}
							littel_endian_size pse_length;
							pse_length.m_byte[0] = PSEPack->PackLength.m_byte[1];
							pse_length.m_byte[1] = PSEPack->PackLength.m_byte[0];

							// 去除填充数据大小得到数据大小
							size_t payload_size = pse_length.length - 2 - 1 - PSEPack->stuffing_length;
							if (payload_size > 0)
							{
								
								// 4 + 2 + 2 + 1 + stuffing 
								const uint8_t *payload = ps +  sizeof(program_stream_e) + PSEPack->stuffing_length;

								
#if 1
								// 判断当前是否nal一个包了
								if (stream_len_ > 4 &&payload[0] == 0x00 && payload[1] == 0X00/*'\00'*/ &&
									payload[2] == 0X00/*'\00'*/ &&
									payload[3] == 0X01/*'\01'*/  
									) 
								{
									libmedia_codec::EncodedImage encode_image;
									encode_image.SetEncodedData(
										libmedia_codec::EncodedImageBuffer::Create(
											h264_stream_,
											stream_len_
										));
									encode_image.SetTimestamp(video_pts_);
									SignalRecvVideoFrame(encode_image);
									//callback_->OnVideoFrame(encode_image);
									stream_len_ = 0;
								}
								
#endif // 
								// pts flag
								if (PSEPack->PackInfo1[1] == '\80')
								{

									// ps +9    ==> 5byte pts 
									int64_t   pts = ps[9];
									pts <<= 8;// sizeof(char);
									pts += ps[10];
									pts <<= 8;// sizeof(char);
									pts += ps[11];
									pts <<= 8;// sizeof(char);
									pts += ps[12];
									pts <<= 8;// sizeof(char);
									pts += ps[13];
									pts <<= 8;// sizeof(char);
									//pts <<= 8;// sizeof(char);
									LIBMPEG_LOG(LS_INFO) << "pts :" << pts;
								}
								if ((PSEPack->PackInfo1[1] & 0xe0) == 0x20) {
									//dts =
									//	pts = get_pts(s->pb, c);
									//len -= 4;
									//if (c & 0x10) {
									//	dts = get_pts(s->pb, -1);
									//	len -= 5;
									//}
									int64_t   pts = ps[9];
									pts <<= 8;// sizeof(char);
									pts += ps[10];
									pts <<= 8;// sizeof(char);
									pts += ps[11];
									pts <<= 8;// sizeof(char);
									pts += ps[12];
									pts <<= 8;// sizeof(char);
									pts += ps[13];
									pts <<= 8;// sizeof(char);
									LIBMPEG_LOG(LS_INFO) << "===========>pts :" << pts;
									video_pts_ = pts / 90;
								}
								else if ((PSEPack->PackInfo1[1] & 0xc0) == 0x80) {
									/* mpeg 2 PES */
									//flags = avio_r8(s->pb);
									//header_len = avio_r8(s->pb);
									//len -= 2;
									//if (header_len > len)
									//	goto error_redo;
									//len -= header_len;
									//if (flags & 0x80) {
									//	dts = pts = get_pts(s->pb, -1);
									//	header_len -= 5;
									//	if (flags & 0x40) {
									//		dts = get_pts(s->pb, -1);
									//		header_len -= 5;
									//	}
									//}
									int64_t   pts = ps[9];
									pts <<= 8;// sizeof(char);
									pts += ps[10];
									pts <<= 8;// sizeof(char);
									pts += ps[11];
									pts <<= 8;// sizeof(char);
									pts += ps[12];
									pts <<= 8;// sizeof(char);
									pts += ps[13];
									pts <<= 8;// sizeof(char);
									LIBMPEG_LOG(LS_INFO) << "===========>pts :" << pts;
									video_pts_ = pts / 90;
								}
								
								// 一帧数据大于 mtu的大小  rtp hreader 12  , rtp payload 1400  就会多个包传输 
								
								//当前包能拷贝的大小
								int32_t  avil_read_byte = (size - len - sizeof(program_stream_e) - PSEPack->stuffing_length);
								if (avil_read_byte < payload_size)
								{
									//fwrite(payload, 1, avil_read_byte, out_file_ptr);
									//fflush(out_file_ptr);
									memcpy(h264_stream_ + stream_len_, payload, avil_read_byte);
									stream_len_ += avil_read_byte;
									read_byte_ = payload_size - avil_read_byte;

								//	RTC_LOG(LS_INFO) << ", payload_size:" << payload_size << ", avil_read_byte: " << avil_read_byte << ",read_byte: " << read_byte_;
									return 0;
								}



								
								//fwrite(payload, 1, payload_size, out_file_ptr);
								//fflush(out_file_ptr);
								memcpy(h264_stream_ + stream_len_, payload, payload_size);
								stream_len_ += payload_size;


#if 0
								memcpy(h264_stream_ + stream_len_, payload, payload_size);
								stream_len_ += payload_size;
								//ps += sizeof(program_stream_e) + PSEPack->stuffing_length;
#endif // stream_len_
							}

							//、、*leftlength = length - pse_length.length - sizeof(pack_start_code) - sizeof(littel_endian_size);
							//if (*leftlength <= 0) return 0;

							ps +=   sizeof(pack_start_code) + sizeof(littel_endian_size) + pse_length.length;
							len += sizeof(pack_start_code) + sizeof(littel_endian_size) + pse_length.length;
							break;
						}
						case 0XC0/*'\c0'*/: // Audio
						{
							program_stream_e* PSEPack = (program_stream_e*)ps;
							if (size - len < 9)
							{
								LIBMPEG_LOG_T_F(LS_ERROR) << "parse PES  video  packet tail small  len:" << len;
								return -1;
							}
							littel_endian_size pse_length;
							pse_length.m_byte[0] = PSEPack->PackLength.m_byte[1];
							pse_length.m_byte[1] = PSEPack->PackLength.m_byte[0];

							// 去除填充数据大小得到数据大小
							size_t playload_size = pse_length.length - 2 - 1 - PSEPack->stuffing_length;
							if (playload_size > 0)
							{
								const uint8_t *payload = ps + sizeof(program_stream_e) + PSEPack->stuffing_length;



								SignalRecvAudioFrame(rtc::CopyOnWriteBuffer(payload, playload_size));
								//if (callback_)
								//{
								//	rtc::Buffer frame(payload, playload_size);
								//	callback_->OnAudioFrame(std::move(frame));
								//}

#if 0
								static std::unique_ptr< libmedia_codec::AudioDecoder>  audio_codec;
								if (!audio_codec)
								{
									audio_codec = std::make_unique<libmedia_codec::AudioDecoder>();
									audio_codec->Configure();
								}
								// 解析adts 头获取aaac的es的数据
								//av_adts_header_parse(payload, playload_size);

								
								static FILE * out_file_ptr = fopen("test_ps.aac", "wb+");
								if (out_file_ptr)
								{
									fwrite(payload, 1, playload_size, out_file_ptr);
									fflush(out_file_ptr);
								}
#endif // 
								//ps += sizeof(program_stream_e) + PSEPack->stuffing_length;
							}

							//、、*leftlength = length - pse_length.length - sizeof(pack_start_code) - sizeof(littel_endian_size);
							//if (*leftlength <= 0) return 0;
							len += sizeof(pack_start_code) + sizeof(littel_endian_size) + pse_length.length;
							ps += sizeof(pack_start_code) + sizeof(littel_endian_size) + pse_length.length;
							break;
						}
						default:
						{
#if 0
							static int32_t count = 0;
							++count;

							std::string file = "./mpeg/parse" + std::to_string(count) + ".ps";
							FILE * out_f = fopen(file.c_str(), "wb+");
							if (out_f)
							{
								fwrite(ps, 1, size-len, out_f);
								fflush(out_f);
								fclose(out_f);
								out_f = nullptr;
							}
#endif // 
							LIBMPEG_LOG_T_F(LS_ERROR) << "parse mpeg  failed !!! (" << start_code->stream_id[0] <<")";
							return -1;
							break;
						}
					}
				}
				else
				{
				//fwrite(data, 1, size, out_file_ptr);
				//fflush(out_file_ptr);
				/*static int32_t count = 0;
				++count;
				
				std::string file = "./test/" +std::to_string(count) + ".ps";
				FILE * out_f = fopen(file.c_str(), "wb+");
				if (out_f)
				{
					fwrite(data, 1, size, out_f);
					fflush(out_f);
					fclose(out_f);
					out_f = nullptr;
				}*/
				LIBMPEG_LOG_T_F(LS_ERROR) << "mpeg ps  start code not find !!!   read_byte_: " << read_byte_ << ", size: " << size << ", len:" << len;;
				return 0;
				}
			}

			return 0;
		}
	}

}
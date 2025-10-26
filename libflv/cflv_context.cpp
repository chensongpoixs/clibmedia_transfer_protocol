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
/***********************************************************************************************
created: 		2025-04-29

author:			chensong

purpose:		http_parser
输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/
#include "libmedia_transfer_protocol/libflv/cflv_context.h"
#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <cstdalign>
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/libflv/amf0.h"
#include "common_video/h264/h264_common.h"
#include "modules/video_coding/include/video_coding.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
namespace libmedia_transfer_protocol
{
	namespace libflv
	{
		namespace {
			static uint8_t flv_audio_only_header[] = {
					0X46, /*'F'*/
					0X4C, /*'L*/
					0X56, /*'V'*/
					0X01, /* version = 1*/
					0X04,
					0X00,
					0X00,
					0X00,
					0X09, /*header seize*/
					0X00,
				0X00,
				0X00,
				0X00
			};

			//	视频flv头


			static uint8_t flv_video_only_header[] = {
				0X46, /*'F'*/
				0X4C, /*'L*/
				0X56, /*'V'*/
				0X01, /* version = 1*/
				0X01,
				0X00,
				0X00,
				0X00,
				0X09, /*header seize*/
				0X00,
				0X00,
				0X00,
				0X00
			};


			//音视频flv头

			static  uint8_t flv_header[] = {
				0X46, /*'F'*/
				0X4C, /*'L*/
				0X56, /*'V'*/
				0X01, /* version = 1*/
				0X05, /*  0000 0101 = has audio & video */
				0X00,
				0X00,
				0X00,
				0X09, /*header seize*/
				//0X00,
				//0X00,
				//0X00,
				//0x00
			};

			static void set_be24(void *p, uint32_t val)
			{
				uint8_t *data = (uint8_t *)p;
				data[0] = val >> 16;
				data[1] = val >> 8;
				data[2] = val;
			}
			static void set_be32(void *p, uint32_t val)
			{
				uint8_t *data = (uint8_t *)p;
				data[0] = val >> 24;
				data[1] = val >> 16;
				data[2] = val >> 8;
				data[3] = val;
			}
			static const char   kflv_muxer[] = "libflv_rtc";


#define FLV_VIDEO_FRAMETYPE_OFFSET   4

			enum {
				FLV_FRAME_KEY = 1 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< key frame (for AVC, a seekable frame)
				FLV_FRAME_INTER = 2 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< inter frame (for AVC, a non-seekable frame)
				FLV_FRAME_DISP_INTER = 3 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< disposable inter frame (H.263 only)
				FLV_FRAME_GENERATED_KEY = 4 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< generated key frame (reserved for server use only)
				FLV_FRAME_VIDEO_INFO_CMD = 5 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< video info/command frame
			};
			enum {
				FLV_CODECID_H263 = 2,
				FLV_CODECID_SCREEN = 3,
				FLV_CODECID_VP6 = 4,
				FLV_CODECID_VP6A = 5,
				FLV_CODECID_SCREEN2 = 6,
				FLV_CODECID_H264 = 7,
				FLV_CODECID_REALH263 = 8,
				FLV_CODECID_MPEG4 = 9,
			};
			// const char   kflv_muxerl[] = "libflv_rtc";
		}
		FlvContext::FlvContext(  libnetwork::Connection* conn, const char * out_flv_file_name)
			:connection_(conn) 
			, out_file_ptr_(NULL)
			, out_buffer_(new uint8_t[1024 * 1024 * 8])
			, start_timestamp_(0)
			, send_sps_(false)
			, sps_("" )
			, pps_( "")
			, send_buffer_(new uint8_t[1024 * 1024 * 8])
			, send_size_(0)
		{
			LIBFLV_LOG_T_F(LS_INFO);
			current_ = out_buffer_;
			prev_packet_size_ = 0;
			// http header
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK \r\n";

			ss << "Access-Control-Allow-Origin:*\r\n";
			ss << "Content-Type: video/x-flv; charset=utf-8\r\n";
			ss << "Connection: Keep-Alive\r\n";
			ss << "\r\n";
			
		 
			if (out_flv_file_name)
			{
				out_file_ptr_ = fopen(out_flv_file_name, "wb+");
			}
			
 
			if (connection_)
			{
				connection_->AsyncSend(ss.str());
			}
			

		}

		FlvContext::~FlvContext()
		{
			LIBFLV_LOG_T_F(LS_INFO);
			if (out_buffer_)
			{
				delete out_buffer_;
				out_buffer_ = NULL;
			}
			if (send_buffer_)
			{
				delete[]send_buffer_;
				send_buffer_ = nullptr;
			}
			if (out_file_ptr_)
			{
				fflush(out_file_ptr_);
				fclose(out_file_ptr_);
				out_file_ptr_ = nullptr;
			}
		}

		
		void FlvContext::SendFlvHeader(bool has_auido, bool has_video)
		{
			
			uint8_t * header = current_;
			//uint8_t   send_buffer[1024] = { 0 };
			int32_t  index = 0;
#if 0
			if (!has_auido)
			{
				memcpy(header+ index, flv_video_only_header, sizeof(flv_video_only_header));
				index +=  sizeof(flv_video_only_header);
			}
			else if (!has_video)
			{
				memcpy(header + index, flv_audio_only_header, sizeof(flv_audio_only_header));
				index +=  sizeof(flv_audio_only_header);
			}
			else
			{
				memcpy(header+ index, flv_header, sizeof(flv_header));
				index +=  sizeof(flv_header);
			}
#else 
			libflv::FLVHeader flv_header = { 0 };

			flv_header.flv[0] = 'F';
			flv_header.flv[1] = 'L';
			flv_header.flv[2] = 'V';
			flv_header.version = libflv::FLVHeader::kFlvVersion;
			flv_header.length = htonl(libflv::FLVHeader::kFlvHeaderLength);
			flv_header.have_video = has_video;
			flv_header.have_audio = has_auido;


			Writer((const uint8_t *)&flv_header, sizeof(libflv::FLVHeader));
#endif 
			//connection_->AsyncSend( rtc::CopyOnWriteBuffer( current_, index ));

			//prev_packet_size_ = index;
			uint8_t * metadata = current_ ;
			uint8_t * ptr = metadata;
			uint8_t *end = ptr + (1024 * 1023);

			uint8_t count = (has_auido ? 5 : 0) + (has_video ? 7 : 0) + 1;
			ptr = AMFWriteString(ptr, end, "onMetaData", 10);
			// value: SCRIPTDATAECMAARRAY
			ptr[0] = AMF_ECMA_ARRAY;
			ptr[1] = (uint8_t)((count >> 24) & 0xFF);;
			ptr[2] = (uint8_t)((count >> 16) & 0xFF);;
			ptr[3] = (uint8_t)((count >> 8) & 0xFF);
			ptr[4] = (uint8_t)(count & 0xFF);
			ptr += 5;


			if (has_auido)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "audiocodecid", 12, 10);
				ptr = AMFWriteNamedDouble(ptr, end, "audiodatarate", 13, 125 /* / 1024.0*/);
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplerate", 15, 44100);
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplesize", 15, 16);
				ptr = AMFWriteNamedBoolean(ptr, end, "stereo", 6, (uint8_t)true);
			}
		
			if (has_video)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "duration", 8, 0 );
				//ptr = AMFWriteNamedDouble(ptr, end, "interval", 8, metadata->interval);
				ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, 7);
				ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, 0 /* / 1024.0*/);
				ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, 25);
				ptr = AMFWriteNamedDouble(ptr, end, "height", 6, 2560);
				ptr = AMFWriteNamedDouble(ptr, end, "width", 5, 1440);
			}
			ptr = AMFWriteNamedString(ptr, end, "encoder", 7, kflv_muxer, strlen(kflv_muxer));
			ptr = AMFWriteObjectEnd(ptr, end);
			 
			WriteFlvTag(libflv::kFlvMsgTypeAMFMeta, metadata, ptr - metadata, 0);
 
			 
			 
			 
		}
		 
		//void FlvContext::SendFlvOnMetaHeader()
		//{
		//}

		/*
		
		# 二、 FLV文件头


		1. 9个字节的TAG，描述版本号， 携带的数据有没有音频， 有没有视频
		2. 音视频有三种组合： 只有音频、只有视频或者音视频都有
		3. FLV文件头总是作为第一个数据先发送
		*/
		bool FlvContext::SendFlvVideoFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp)
		{
			
			std::vector<webrtc::H264::NaluIndex> nalus = webrtc::H264::FindNaluIndices(
				frame.data(), frame.size());
			for (int32_t nal_index = 0; nal_index < nalus.size(); ++nal_index)
			{
				webrtc::NaluInfo nalu;
				nalu.type = frame.data()[nalus[nal_index].payload_start_offset] & 0x1F;
				nalu.sps_id = -1;
				nalu.pps_id = -1;
				switch (nalu.type) {
				case webrtc::H264::NaluType::kSps: {
					sps_ =  (std::string((const char *)(frame.data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					 
					break;
				}
				case webrtc::H264::NaluType::kPps: {

					pps_ =  (std::string((char *)(frame.data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					break;
				}
				case webrtc::H264::NaluType::kIdr:
				{


					if (!send_sps_)
					{
						send_sps_ = true;
						LIBFLV_LOG_T_F(LS_INFO) << "send decoder config ...";
						WriteConfigPacket();
						start_timestamp_ = timestamp;
					}
					uint8_t * buffer = out_buffer_;

					uint8_t *ptr = buffer;
					*ptr = FLV_CODECID_H264;
					*ptr++ |= FLV_FRAME_KEY;
					//auto flags = (uint8_t)7;
					//flags |= ((uint8_t)1 << 4);
					//std::string  packet;

					// header
					//uint8_t dd = 0;
					//packet.append((char)flags);
					//packet.append((char)dd);
					// avio_w8(pb, par->codec_tag | FLV_FRAME_KEY); // flags
					// avio_w8(pb, 0); // AVC sequence header
					// avio_wb24(pb, 0); // composition time
					*ptr++ = 1;
					//*ptr++ = 0;
					//*ptr++ = 0;
					//*ptr++ = 0;
					set_be24(ptr, start_timestamp_); 
					ptr += 3;
					// avcc
					// sps
					set_be32(ptr, sps_.size());
					ptr += 4;
					memcpy(ptr, sps_.c_str(), sps_.size());
					ptr += sps_.size();
					// pps
					set_be32(ptr, pps_.size());
					ptr += 4;
					memcpy(ptr, pps_.c_str(), pps_.size());
					ptr += pps_.size();

					// idr
					set_be32(ptr, nalus[nal_index].payload_size);
					ptr += 4;
					memcpy(ptr, frame.data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					//uint8_t  * new_data = (uint8_t*)(encoded_image->data() + nalus[nal_index].payload_start_offset - 2) ;
					//new_data[0] = 2;
					//new_data[1] = 1;
					//LIBFLV_LOG_T_F(LS_INFO) << "send idr info ...";
					WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer,
						ptr - buffer, timestamp - start_timestamp_);
					//LIBFLV_LOG_T_F(LS_INFO) << "send idr info -->";
					break;
				}
				case webrtc::H264::NaluType::kSlice:  						 // Slices below don't contain SPS or PPS ids.
				case webrtc::H264::NaluType::kAud:
				case webrtc::H264::NaluType::kEndOfSequence:
				case webrtc::H264::NaluType::kEndOfStream:
				case webrtc::H264::NaluType::kFiller:
				case webrtc::H264::NaluType::kSei:
				case webrtc::H264::NaluType::kStapA:
				case webrtc::H264::NaluType::kFuA:
				{
					if (!send_sps_)
					{
						//必须先发送sps pps idr 信息  decoder 解码信息  
						continue;
					}
					uint8_t * buffer = out_buffer_;

					uint8_t *ptr = buffer;
					*ptr = FLV_CODECID_H264;
					*ptr++ |= FLV_FRAME_INTER;



					//auto flags = (uint8_t)7;
					//flags |= ((uint8_t)1 << 4);
					//std::string  packet;

					// header
					//uint8_t dd = 0;
					//packet.append((char)flags);
					//packet.append((char)dd);
					// avio_w8(pb, par->codec_tag | FLV_FRAME_KEY); // flags
					// avio_w8(pb, 0); // AVC sequence header
					// avio_wb24(pb, 0); // composition time
					//avio_w8(pb, 1); // AVC NALU
					//avio_wb24(pb, pkt->pts - pkt->dts);
					 *ptr++ = 1;
					// *ptr++ = 0;
					// *ptr++ = 0;
					// *ptr++ = 0;
					set_be24(ptr, timestamp-  start_timestamp_);
					ptr += 3;
					// idr
					set_be32(ptr, nalus[nal_index].payload_size);
					ptr += 4;
					memcpy(ptr, frame.data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					//uint8_t  * new_data = (uint8_t*)(encoded_image->data() + nalus[nal_index].payload_start_offset - 2) ;
					//new_data[0] = 2;
					//new_data[1] = 1;
					//LIBFLV_LOG_T_F(LS_INFO) << "send sclie  info ...";
					WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer,
						ptr - buffer, timestamp - start_timestamp_);
					 
					break;
				}
				default: {
					break;
				}

				}
			}

			return true;
		}
		bool FlvContext::SendFlvAudioFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp)
		{


			// sps
			// pps 
			// idr 
			// 00 00 00 01 
			// 不使用上面的模式


			uint32_t packet_size = frame.size();
			uint8_t * send_buffer = current_;
			uint32_t   index_size = 0;
			uint8_t * p = (uint8_t *)&prev_packet_size_;
			//  << "flv  header previous size: " << prev_packet_size_;
			//  << "flv  header hex : " <<  

			// (大端对齐)
			/*memcpy(current_, p, sizeof(uint32_t));
			current_ += 4;*/
			//记录上一个tag的包数据的大小
			send_buffer[index_size++] = p[3];
			//current_++;
			send_buffer[index_size++] = p[2];
			//current_++;
			send_buffer[index_size++] = p[1];
			//current_++;
			send_buffer[index_size++] = p[0];
			//current_++;
			// 包类型 
			send_buffer[index_size++] = kFlvMsgTypeAudio;// 

			//包长度 3个字节
			
			p = (uint8_t *)&packet_size;
			send_buffer[index_size++] = p[2];
			send_buffer[index_size++] = p[1];
			send_buffer[index_size++] = p[0];
			(void)(packet_size);
			p = (uint8_t *)&timestamp;
			send_buffer[index_size++] = p[2];
			send_buffer[index_size++] = p[1];
			send_buffer[index_size++] = p[0];
			send_buffer[index_size++] = 0; //固定输入 '0';


			send_buffer[index_size++] = 0;
			send_buffer[index_size++] = 0;
			send_buffer[index_size++] = 0;
			{
			  // vido type
				//send_buffer[index_size++] = 0x17;
				//send_buffer[index_size++] = 0;
			}
			//记录tag的包的大小给下一个包使用
			prev_packet_size_ = packet_size + 11;

		 
			 if (out_file_ptr_)
			 {
				 fwrite(current_, 1, index_size, out_file_ptr_);
				 fwrite(frame.data(), 1, frame.size(), out_file_ptr_);

				 fflush(out_file_ptr_);
			 }
 
			 if (connection_)
			 { 
				 connection_->AsyncSend(rtc::CopyOnWriteBuffer(current_, index_size));


				 connection_->AsyncSend(rtc::CopyOnWriteBuffer(frame));
			 }
			
 
			return true;
		}

		

		void FlvContext::WriteConfigPacket()
		{

			uint8_t * buffer = out_buffer_;

			uint8_t *ptr = buffer;
			*ptr = FLV_CODECID_H264;
			*ptr++ |= FLV_FRAME_KEY;
			//config 编码信息设置为0 
			*ptr++ = 0;
			*ptr++ = 0;
			*ptr++ = 0;
			*ptr++ = 0;
			// cts 
			// AVCDecoderConfigurationRecord start
			std::string extra_data;
			{

				/*
				configurationVersion	8	版本号，总是1
				AVCProfileIndication	8	sps[1]
				profile_compatibility	8	sps[2]
				AVCLevelIndication	8	sps[3]
				configurationVersion,AVCProfileIndication,profile_compatibility,AVCLevelIndication：都是一个字节，具体的内容由解码器去理解。
				lengthSizeMinusOne：unit_length长度所占的字节数减1，也即lengthSizeMinusOne的值+1才是unit_length所占用的字节数。
				numOfSequenceParameterSets：sps的个数
				sequenceParameterSetLength：sps内容的长度
				sequenceParameterSetNALUnit：sps的内容
				numOfPictureParameterSets：pps的个数
				pictureParameterSetLength：pps内容的长度
				pictureParameterSetNALUnit：pps的内容
				*/
				// AVCDecoderConfigurationRecord start
				extra_data.push_back(1); // version
			//	*ptr++ = 1;
				extra_data.push_back(sps_[1]); // profile
				//*ptr++ = sps_[1];
				extra_data.push_back(sps_[2]); // compat
				//*ptr++ = sps_[2];
				extra_data.push_back(sps_[3]); // level
				//*ptr++ = sps_[3];
				extra_data.push_back((char)0xff); // 6 bits reserved + 2 bits nal size length - 1 (11)
				//*ptr++ = 0xff;
				extra_data.push_back((char)0xe1); // 3 bits reserved + 5 bits number of sps (00001)
				//*ptr++ = 0xe1;
				// sps
				uint16_t size = (uint16_t)sps_.size();
				size = htons(size);
				extra_data.append((char *)&size, 2);
				extra_data.append(sps_);
				
				// pps
				extra_data.push_back(1); // version
				size = (uint16_t)pps_.size();
				size = htons(size);
				extra_data.append((char *)&size, 2);
				extra_data.append(pps_);

			}

			// memcpy()
			//packet.append(extra_data);
			memcpy(ptr, extra_data.c_str(), extra_data.size());
			ptr += extra_data.size();
			WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer, ptr - buffer, 0);
			 
		}


		void FlvContext::WriteFlvTag(uint8_t type, const uint8_t * data, int32_t size, int64_t time_stamp)
		{

			
			libflv::FlvTagHeader header;
			header.type = type;
			set_be24(header.data_size, (uint32_t)size);
			header.timestamp_ex = (time_stamp >> 24) & 0xff;
			set_be24(header.timestamp, time_stamp & 0xFFFFFF);


			//tag header
			std::string tag_header;
			tag_header.append((char *)&header, sizeof(header));
			Writer((const uint8_t *)tag_header.c_str(), tag_header.size()); ///

			//tag data
			Writer(data, size);

			//PreviousTagSize
			uint32_t PreviousTag_Size = htonl((uint32_t)(size + sizeof(header)));
			std::string PreviousTagSize;
			PreviousTagSize.append((char *)&PreviousTag_Size, 4);
			Writer((const uint8_t *)PreviousTagSize.c_str(), PreviousTagSize.size(), true); /// (obtainBuffer(), false);

		}

		void FlvContext::Writer(const uint8_t * data, int32_t size, bool fflsh)
		{
			if (out_file_ptr_)
			{
				fwrite(data, 1, size, out_file_ptr_);


				fflush(out_file_ptr_);
			}
			memcpy(send_buffer_ + send_size_, data, size);
			send_size_ += size;
			if (fflsh)
			{
				if (connection_)
				{

					connection_->AsyncSend(rtc::CopyOnWriteBuffer(send_buffer_, send_size_));

				}
				
				send_size_ = 0;
			}
			
		}
		 
	}
}
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
#include "libmedia_transfer_protocol/libflv/amf0.h"
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



			static const char   kflv_muxer[] = "libflv_rtc";
		}
		FlvContext::FlvContext(  libnetwork::Connection* conn )
			:connection_(conn) 
#if TEST_HTTP_FLV
			, out_file_ptr(nullptr)
#endif  
		{
			current_ = out_buffer_;
			prev_packet_size_ = 0;
			// http header
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK \r\n";

			ss << "Access-Control-Allow-Origin:*\r\n";
			ss << "Content-Type: video/x-flv; charset=utf-8\r\n";
			ss << "Connection: Keep-Alive\r\n";
			ss << "\r\n";
			//connection_->AsyncSend());
			//rtc::CopyOnWriteBuffer   tem_flv_header;
			//tem_flv_header.AppendData(ss.str());
			connection_->AsyncSend(ss.str());
#if TEST_HTTP_FLV
			std::string file_name_flv = "test.flv.flv";
			out_file_ptr = fopen(file_name_flv.c_str(), "wb+");
#endif 
		}



		
		void FlvContext::SendFlvHeader(bool has_auido, bool has_video)
		{
			
			uint8_t * header = current_;
			//uint8_t   send_buffer[1024] = { 0 };
			int32_t  index = 0;
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
			//connection_->AsyncSend( rtc::CopyOnWriteBuffer( current_, index ));

			//prev_packet_size_ = index;
			uint8_t * metadata = current_ + index;
			uint8_t * ptr = current_ + index +13;
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
			uint32_t meta_data_length = ptr - metadata -13  -2;
			// tag meta header data 
			memset(metadata, '\0', 13);
			metadata[4+0] = kFlvMsgTypeAMFMeta; // tag type 
			metadata[4+1] = meta_data_length/65536; // length;
			metadata[4+2] = (meta_data_length % 65536) /256; // length;
			metadata[4+3] = meta_data_length%256; // length;
			connection_->AsyncSend(rtc::CopyOnWriteBuffer(current_,   ptr - current_));
#if TEST_HTTP_FLV
			if (out_file_ptr)
			{
				fwrite(current_, 1, ptr - current_, out_file_ptr);
				fflush(out_file_ptr);
			}
#endif //#if TEST_HTTP_FLV
			//uint8_t* ptr, *end;
			//	uint32_t count;
			//
			//	if (!metadata) return -1;
			//
			//	if (flv->capacity < 1024)
			//	{
			//		if (0 != flv_muxer_alloc(flv, 1024))
			//			return -ENOMEM;
			//	}
			//
			//	ptr = flv->ptr;
			//	end = flv->ptr + flv->capacity;
			//	count = (metadata->audiocodecid ? 5 : 0) + (metadata->videocodecid ? 7 : 0) + 1;
			//
			//	// ScriptTagBody
			//
			//	// name
			//	ptr = AMFWriteString(ptr, end, "onMetaData", 10);
			//
			//	// value: SCRIPTDATAECMAARRAY
			//	ptr[0] = AMF_ECMA_ARRAY;
			//	ptr[1] = (uint8_t)((count >> 24) & 0xFF);;
			//	ptr[2] = (uint8_t)((count >> 16) & 0xFF);;
			//	ptr[3] = (uint8_t)((count >> 8) & 0xFF);
			//	ptr[4] = (uint8_t)(count & 0xFF);
			//	ptr += 5;
			//
			//	if (metadata->audiocodecid)
			//	{
			//		ptr = AMFWriteNamedDouble(ptr, end, "audiocodecid", 12, metadata->audiocodecid);
			//		ptr = AMFWriteNamedDouble(ptr, end, "audiodatarate", 13, metadata->audiodatarate /* / 1024.0*/);
			//		ptr = AMFWriteNamedDouble(ptr, end, "audiosamplerate", 15, metadata->audiosamplerate);
			//		ptr = AMFWriteNamedDouble(ptr, end, "audiosamplesize", 15, metadata->audiosamplesize);
			//		ptr = AMFWriteNamedBoolean(ptr, end, "stereo", 6, (uint8_t)metadata->stereo);
			//	}
			//
			//	if (metadata->videocodecid)
			//	{
			//		ptr = AMFWriteNamedDouble(ptr, end, "duration", 8, metadata->duration);
			//		ptr = AMFWriteNamedDouble(ptr, end, "interval", 8, metadata->interval);
			//		ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, metadata->videocodecid);
			//		ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, metadata->videodatarate /* / 1024.0*/);
			//		ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, metadata->framerate);
			//		ptr = AMFWriteNamedDouble(ptr, end, "height", 6, metadata->height);
			//		ptr = AMFWriteNamedDouble(ptr, end, "width", 5, metadata->width);
			//	}
			//
			//	ptr = AMFWriteNamedString(ptr, end, "encoder", 7, FLV_MUXER, strlen(FLV_MUXER));
			//	ptr = AMFWriteObjectEnd(ptr, end);
			//
			//	return flv->handler(flv->param, FLV_TYPE_SCRIPT, flv->ptr, ptr - flv->ptr, 0);







		}
		 
		/*
		
		# 二、 FLV文件头


		1. 9个字节的TAG，描述版本号， 携带的数据有没有音频， 有没有视频
		2. 音视频有三种组合： 只有音频、只有视频或者音视频都有
		3. FLV文件头总是作为第一个数据先发送
		*/
		bool FlvContext::SendFlvVideoFrame(const rtc::CopyOnWriteBuffer & frame, uint32_t timestamp)
		{
			uint32_t packet_size = frame.size()+2;
			uint8_t * send_buffer = current_; 
			uint32_t   index_size = 0;
			uint8_t * p = (uint8_t *)&prev_packet_size_;
			//  << "flv  header previous size: " << prev_packet_size_;
			//  << "flv  header hex : " <<  

			// (大端对齐)
			/*memcpy(current_, p, sizeof(uint32_t));
			current_ += 4;*/
			send_buffer[index_size++] = p[3];
			//current_++;
			send_buffer[index_size++] = p[2];
			//current_++;
			send_buffer[index_size++] = p[1];
			//current_++;
			send_buffer[index_size++] = p[0];
			//current_++;
			// 包类型 
			send_buffer[index_size++] = kFlvMsgTypeVideo;// 

			//包长度 3个字节
			
			
			p = (uint8_t *)&packet_size;
			send_buffer[index_size++] = p[2];
			send_buffer[index_size++] = p[1];
			send_buffer[index_size++] = p[0];
		//	(void)(packet_size);
			 
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
				send_buffer[index_size++] = 0x17;
				send_buffer[index_size++] = 0;
			}
			//记录tag的包的大小给下一个包使用
			prev_packet_size_ = packet_size + 11  ;

			 connection_->AsyncSend(rtc::CopyOnWriteBuffer(current_, index_size));
			 

			 connection_->AsyncSend(rtc::CopyOnWriteBuffer(frame));
#if TEST_HTTP_FLV
			 if (out_file_ptr)
			 {
				 fwrite(current_, 1, index_size, out_file_ptr);
				 fwrite(frame.data(), 1, frame.size(), out_file_ptr);

				 fflush(out_file_ptr);
			 }
#endif //#if TEST_HTTP_FLV
			return true;
		}
		bool FlvContext::SendFlvAudioFrame(const rtc::CopyOnWriteBuffer & frame, uint32_t timestamp)
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

			 connection_->AsyncSend(rtc::CopyOnWriteBuffer(current_, index_size));


			 connection_->AsyncSend(rtc::CopyOnWriteBuffer(frame));
#if TEST_HTTP_FLV
			 if (out_file_ptr)
			 {
				 fwrite(current_, 1, index_size, out_file_ptr);
				 fwrite(frame.data(), 1, frame.size(), out_file_ptr);

				 fflush(out_file_ptr);
			 }
#endif //#if TEST_HTTP_FLV
			return true;
		}
		 
	}
}
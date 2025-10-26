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

#ifndef _C_FLV_CONTEXT______
#define _C_FLV_CONTEXT______


#include <cstdint>
#include <memory> 
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
 
#include <functional>
#include <memory>
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/system/arch.h"
namespace libmedia_transfer_protocol
{
	namespace libflv
	{
		enum FlvMsgType
		{
			kFlvMsgTypeAudio = 8,
			kFlvMsgTypeVideo = 9, 
			kFlvMsgTypeAMFMeta = 18, 
		};









#pragma pack(push, 1)

		class FLVHeader {
		public:
			static constexpr uint8_t kFlvVersion = 1;
			static constexpr uint8_t kFlvHeaderLength = 9;
			//FLV
			char flv[3];
			//File version (for example, 0x01 for FLV version 1)
			uint8_t version;
#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )


			// 保留,置0  [AUTO-TRANSLATED:46985374]
			// Preserve, set to 0
			uint8_t : 5;
			// 是否有音频  [AUTO-TRANSLATED:9467870a]
			// Whether there is audio
			uint8_t have_audio : 1;
			// 保留,置0  [AUTO-TRANSLATED:46985374]
			// Preserve, set to 0
			uint8_t : 1;
					// 是否有视频  [AUTO-TRANSLATED:42d0ed81]
					// Whether there is video
					uint8_t have_video : 1;
#elif  defined(  WEBRTC_ARCH_BIG_ENDIAN)
			// 是否有视频  [AUTO-TRANSLATED:42d0ed81]
			// Whether there is video
			uint8_t have_video : 1;
			// 保留,置0  [AUTO-TRANSLATED:46985374]
			// Preserve, set to 0
			uint8_t : 1;
					  // 是否有音频  [AUTO-TRANSLATED:9467870a]
					  // Whether there is audio
					  uint8_t have_audio : 1;
					  // 保留,置0  [AUTO-TRANSLATED:46985374]
					  // Preserve, set to 0
					  uint8_t : 5;
#endif
								// The length of this header in bytes,固定为9  [AUTO-TRANSLATED:126988fc]
								// The length of this header in bytes, fixed to 9
								uint32_t length;
								// 固定为0  [AUTO-TRANSLATED:d266c0a7]
								// Fixed to 0
								uint32_t previous_tag_size0;
		};


		class FlvTagHeader {
		public:
			uint8_t type = 0;
			uint8_t data_size[3] = { 0 };
			uint8_t timestamp[3] = { 0 };
			uint8_t timestamp_ex = 0;
			uint8_t streamid[3] = { 0 }; /* Always 0. */
		};
		struct RtmpVideoHeaderEnhanced {
#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )
			uint8_t enhanced : 1;
			uint8_t frame_type : 3;
			uint8_t pkt_type : 4;
			uint32_t fourcc;
#elif defined( WEBRTC_ARCH_BIG_ENDIAN   )
			uint8_t pkt_type : 4;
			uint8_t frame_type : 3;
			uint8_t enhanced : 1;
			uint32_t fourcc;
#endif
		};

		struct RtmpVideoHeaderClassic {
#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )
			uint8_t frame_type : 4;
			uint8_t codec_id : 4;
			uint8_t h264_pkt_type;
#elif defined( WEBRTC_ARCH_BIG_ENDIAN   )
			uint8_t codec_id : 4;
			uint8_t frame_type : 4;
			uint8_t h264_pkt_type;
#endif
		};

#pragma pack(pop)



		class FlvContext
		{
		public:

		  explicit	FlvContext(  
			  libnetwork::Connection* conn, const char * out_flv_file_name  = nullptr);
 
			

		  virtual ~FlvContext();


		public:
			void SendFlvHeader(bool has_auido, bool has_video);
			
			//void SetSps(const std::string & sps);
			//void SetPps(const std::string & pps);
		//	void SendFlvOnMetaHeader();
			bool SendFlvVideoFrame(const rtc::CopyOnWriteBuffer & frame, uint32_t timestamp);
			bool SendFlvAudioFrame(const rtc::CopyOnWriteBuffer & frame, uint32_t timestamp);
			
			 

		private:

			//void WriteFlvHeader();
			void WriteFlvTag(uint8_t type, const uint8_t * data, int32_t size, int64_t timestamp);
			//void WriteMetaData();
			void WriteConfigPacket();
			void Writer(const uint8_t * data, int32_t size, bool fflsh = false);
		private: 

			
			libnetwork::Connection *          connection_;
			FILE *out_file_ptr_;

			uint32_t                  prev_packet_size_; //记录上一个tag的包的大小
			//std::string					http_header_;
			uint8_t *out_buffer_  { nullptr };
			uint8_t * current_{ nullptr }; 
 
		 	
			bool                  send_sps_;
			//rtc::CopyOnWriteBuffer  sps_;
			//rtc::CopyOnWriteBuffer  pps_;
			std::string            sps_;
			std::string            pps_;

			uint32_t               start_timestamp_;

			uint8_t * send_buffer_{nullptr};
			int32_t   send_size_;
 
		//	MMediaHandler*   handler_;
		};
	}
}

#endif  
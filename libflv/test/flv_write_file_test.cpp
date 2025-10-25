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
 created: 		2025-10-25

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
#include "libmedia_transfer_protocol/libflv/test/flv_write_file_test.h"
 

namespace libmedia_transfer_protocol
{
	namespace  libflv_test {




		FlvWriterFileTest::FlvWriterFileTest(const char * out_file_name)
			: write_flv_header_(false)
			, flv_context_(new  libmedia_transfer_protocol::libflv::FlvContext(nullptr, out_file_name))
		{
			x264_encoder_ = std::make_unique<libmedia_codec::X264Encoder>();
			x264_encoder_->SignalVideoEncodedImage.connect(this, &FlvWriterFileTest::OnVideoEncode);
			x264_encoder_->Start();
			video_encoder_thread_ = rtc::Thread::Create();
			video_encoder_thread_->SetName("video_encoder_thread", NULL);
			video_encoder_thread_->Start();

			capturer_track_source_ = libcross_platform_collection_render::CapturerTrackSource::Create(false);
			capturer_track_source_->set_catprue_callback(x264_encoder_.get(), video_encoder_thread_.get());
			capturer_track_source_->StartCapture();
		}

		FlvWriterFileTest::~FlvWriterFileTest()
		{
			video_encoder_thread_->Invoke<void>(RTC_FROM_HERE, [this]() {
				capturer_track_source_->Stop();
				x264_encoder_->SignalVideoEncodedImage.disconnect_all();
				x264_encoder_->Stop();
			});
			video_encoder_thread_->Stop();

			flv_context_.reset();
		}


		void   FlvWriterFileTest::OnVideoEncode(std::shared_ptr<libmedia_codec::EncodedImage> encoded_image)
		{
		
			if (!write_flv_header_)
			{
				write_flv_header_ = true;
				flv_context_->SendFlvHeader(true, true);
			}

			static FILE  *out_file_ptr = fopen("test.h264", "wb+");
			if (out_file_ptr)
			{
				fwrite(encoded_image->data(), 1, encoded_image->size(), out_file_ptr);
				fflush(out_file_ptr);
			}


			flv_context_->SendFlvVideoFrame(rtc::CopyOnWriteBuffer(encoded_image->data(), encoded_image->size()), encoded_image->Timestamp());
		}

	}
}

 
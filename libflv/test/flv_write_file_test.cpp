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
 ��Ӯ����Ҫ���𰸶�������ʲô�������Ҫ��

 �����ߣ��ٴ�֮����Ҳ��Ψ�з������ܣ�����������ʱ����ʱ����Ӣ�ۣ�Ӣ�۴�����ʱ�����������˵�����񣬿��㱾�����ٰ��� �����ã��Լ�������ͼ��顣


 �ҿ��ܻ������ܶ���ˣ������ǽ���2��Ĺ��£�����д�ɹ��»��ɸ裬����ѧ���ĸ���������������
 Ȼ�󻹿�����һ����������һ�������ҵĹ�������ܻᱻ��˧����������ֻᱻ��������ں���������ĳ�������ҹ������ȫ����͸Ȼ��Ҫ������С��ס�������ϵ�ʪ�·���
 3Сʱ���������������ʵ��������ҵĹ�������Ը���򸸻���顣�Ҳ����Ѹ��������Ǵ�Խ�����������ǣ���ʼ��Ҫ�ص��Լ������硣
 Ȼ���ҵ���Ӱ������ʧ���ҿ������������ˮ������ȴû���κ�ʹ�࣬�Ҳ�֪����ԭ���ҵ��ı������ˣ�������ȫ�����ԭ�򣬾���Ҫ�һ��Լ��ı��ġ�
 �����ҿ�ʼ����Ѱ�Ҹ��ָ���ʧȥ�ĵ��ˣ��ұ��һ��שͷ��һ������һ��ˮ��һ����ƣ�ȥ�����Ϊʲô��ʧȥ�Լ��ı��ġ�
 �ҷ��֣��ճ����ı��������Ļ��ڣ������ģ����ǵı��ľͻ���ʧ���յ��˸��ֺڰ�֮�����ʴ��
 ��һ�����ۣ������ʺͱ��ߣ�����ί����ʹ�࣬�ҿ���һֻֻ���ε��֣������ǵı��ĳ��飬�ɱΣ�͵�ߣ���Ҳ�ز������˶���ߡ�
 �ҽ����������֡��������Ǻ�����ͬ�ڵļ��� �����Ҳ������£�����ϸ�����Լ�ƽ����һ�� Ѱ�ұ������ֵĺۼ���
 �����Լ��Ļ��䣬һ�����ĳ�����������������֣��ҵı��ģ�����д�����ʱ�򣬻������
 ��������Ȼ����������ҵ�һ�У�д��������ұ��Ļع����÷�ʽ���һ�û�ҵ��������֣��������ţ�˳�������������һ����˳�����ϣ�������������
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

 
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
#include "libmedia_transfer_protocol/libflv/cflv_context.h"
#include "common_video/h264/h264_common.h"
#include "libmedia_transfer_protocol/libflv/amf0.h"
namespace libmedia_transfer_protocol
{
	namespace  libflv_test {
		namespace {
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

		}



		FlvWriterFileTest::FlvWriterFileTest(const char * out_file_name)
			: write_flv_header_(false)
			, flv_context_(new  libmedia_transfer_protocol::libflv::FlvContext(nullptr, out_file_name))
			 
			, out_flv_file_ptr_(nullptr)
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
			out_flv_file_ptr_ = fopen("test_fffw.flv", "wb+");
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
				WriteFlvHeader();
				WriteMetaData();
			}
#if 1
			static FILE  *out_file_ptr = fopen("test.h264", "wb+");
			if (out_file_ptr)
			{
				fwrite(encoded_image->data(), 1, encoded_image->size(), out_file_ptr);
				fflush(out_file_ptr);
			}
#endif //

			std::vector<webrtc::H264::NaluIndex> nalus = webrtc::H264::FindNaluIndices(
				encoded_image->data(), encoded_image->size());
			for (int32_t nal_index = 0; nal_index < nalus.size(); ++nal_index)
			{  
				webrtc::NaluInfo nalu;
				nalu.type = encoded_image->data()[nalus[nal_index].payload_start_offset] & 0x1F;
				nalu.sps_id = -1;
				nalu.pps_id = -1;
				switch (nalu.type) {
				case webrtc::H264::NaluType::kSps: { 
					sps_ = (std::string((char *)(encoded_image->data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					break;
				}
				case webrtc::H264::NaluType::kPps: {
					 
					pps_= (std::string((char *)(encoded_image->data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					break;
				}
				case webrtc::H264::NaluType::kIdr:
				{


					static bool  ssp_f = false;
					if (!ssp_f)
					{
						ssp_f = true;
						WriteConfigPacket();
					} 

					uint8_t * buffer = new uint8_t[1024 * 1024];

					uint8_t *ptr = buffer;
					*ptr = 7;
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
					*ptr++ = 0;
					*ptr++ = 0;
					*ptr++ = 0;



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
					memcpy(ptr, encoded_image->data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					//uint8_t  * new_data = (uint8_t*)(encoded_image->data() + nalus[nal_index].payload_start_offset - 2) ;
					//new_data[0] = 2;
					//new_data[1] = 1;
					WriteFlvTag(9, buffer,
						ptr - buffer, encoded_image->Timestamp());
					if (buffer)
					{
						delete[] buffer;
						buffer = nullptr;
					}
					
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
					uint8_t * buffer = new uint8_t[1024 * 1024];

					uint8_t *ptr = buffer;
					*ptr = 7;
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
					*ptr++ = 0;
					*ptr++ = 0;
					*ptr++ = 0;
					// idr
					set_be32(ptr, nalus[nal_index].payload_size);
					ptr += 4;
					memcpy(ptr, encoded_image->data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					//uint8_t  * new_data = (uint8_t*)(encoded_image->data() + nalus[nal_index].payload_start_offset - 2) ;
					//new_data[0] = 2;
					//new_data[1] = 1;
					WriteFlvTag(9, buffer,
						ptr - buffer, encoded_image->Timestamp());
					if (buffer)
					{
						delete[] buffer;
						buffer = nullptr;
					}
					break;
				}
				default: { 
					break;
				}

				}
			}
			

			flv_context_->SendFlvVideoFrame(rtc::CopyOnWriteBuffer(encoded_image->data(), encoded_image->size()), encoded_image->Timestamp());
		}

		void FlvWriterFileTest::WriteFlvHeader()
		{
			libflv::FLVHeader flv_header = {0};

			flv_header.flv[0] = 'F';
			flv_header.flv[1] = 'L';
			flv_header.flv[2] = 'V';
			flv_header.version = libflv::FLVHeader::kFlvVersion;
			flv_header.length = htonl(libflv::FLVHeader::kFlvHeaderLength);
			flv_header.have_video =  true;
			flv_header.have_audio = true;


			Writer((const uint8_t *)&flv_header, sizeof(libflv::FLVHeader));
			
		}

		void FlvWriterFileTest::WriteFlvTag(uint8_t type,  const uint8_t * data, int32_t size, int64_t time_stamp)
		{
			libflv::FlvTagHeader header;
			header.type = type;
			set_be24(header.data_size, (uint32_t)size);
			header.timestamp_ex = (time_stamp >> 24) & 0xff;
			set_be24(header.timestamp, time_stamp & 0xFFFFFF);

			//tag header
			std::string tag_header;
			tag_header.append((char *)&header, sizeof(header));
			Writer((const uint8_t *)tag_header.c_str(), tag_header.size()); /// (obtainBuffer(), false);

			//tag data
			Writer(data, size);

			//PreviousTagSize
			uint32_t PreviousTag_Size = htonl((uint32_t)(size + sizeof(header)));
			std::string PreviousTagSize;
			PreviousTagSize.append((char *)&PreviousTag_Size, 4);
			Writer((const uint8_t *)PreviousTagSize.c_str(), PreviousTagSize.size()); /// (obtainBuffer(), false);

		}

		void FlvWriterFileTest::WriteMetaData()
		{
			uint8_t * buffer = new uint8_t [1024 * 1024];
			uint8_t * metadata = buffer;
			uint8_t * ptr = buffer  ;
			uint8_t *end = ptr + (1024 * 1023);
			bool has_auido = true;
			bool has_video = true;
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
				ptr = AMFWriteNamedDouble(ptr, end, "duration", 8, 0);
				//ptr = AMFWriteNamedDouble(ptr, end, "interval", 8, metadata->interval);
				ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, 7);
				ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, 0 /* / 1024.0*/);
				ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, 25);
				ptr = AMFWriteNamedDouble(ptr, end, "height", 6, 1920);
				ptr = AMFWriteNamedDouble(ptr, end, "width", 5, 1080);
			}
			ptr = AMFWriteNamedString(ptr, end, "encoder", 7, kflv_muxer, strlen(kflv_muxer));
			ptr = AMFWriteObjectEnd(ptr, end);
			

			WriteFlvTag(18, metadata, ptr - metadata, 0);
			//uint32_t meta_data_length = ptr - metadata   ;
			//// tag meta header data 
			//memset(metadata, '\0', 13);
			//metadata[4 + 0] = libmedia_transfer_protocol::libflv:: kFlvMsgTypeAMFMeta; // tag type 
			//metadata[4 + 1] = meta_data_length / 65536; // length;
			//metadata[4 + 2] = (meta_data_length % 65536) / 256; // length;
			//metadata[4 + 3] = meta_data_length % 256; // length;


			//if (out_file_ptr_)
			//{
			//	fwrite(current_, 1, ptr - current_, out_file_ptr_);
			//	fflush(out_file_ptr_);
			//}

		}

		void FlvWriterFileTest::WriteConfigPacket()
		{

			uint8_t * buffer = new uint8_t [1024 * 1024];

			uint8_t *ptr = buffer;
			*ptr = 7;
			*ptr++ |= (uint8_t)1 << 4;
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
			//config ������Ϣ����Ϊ0 
			*ptr++ = 0;
			*ptr++ = 0;
			*ptr++ = 0;
			*ptr++ = 0;
			// cts
			//packet.append("\x0\x0\x0", 3);
			// AVCDecoderConfigurationRecord start
			std::string extra_data;
			{
			
				
				// AVCDecoderConfigurationRecord start
				extra_data.push_back(1); // version
				extra_data.push_back(sps_[1]); // profile
				extra_data.push_back(sps_[2]); // compat
				extra_data.push_back(sps_[3]); // level
				extra_data.push_back((char)0xff); // 6 bits reserved + 2 bits nal size length - 1 (11)
				extra_data.push_back((char)0xe1); // 3 bits reserved + 5 bits number of sps (00001)
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
			WriteFlvTag(9, buffer, ptr - buffer, 0);
			delete []buffer;
		}

		void FlvWriterFileTest::Writer(const uint8_t * data, int32_t size)
		{
			if (out_flv_file_ptr_)
			{
				::fwrite(data, 1, size, out_flv_file_ptr_);
				::fflush(out_flv_file_ptr_);
			}
		}

	}
}

 
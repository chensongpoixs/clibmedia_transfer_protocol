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
				   date:  2025-10-07



 ******************************************************************************/
#include "libmedia_transfer_protocol/video_receive_stream.h"

#include "rtc_base/strings/string_builder.h"

namespace libmedia_transfer_protocol {

	VideoReceiveStream::VideoReceiveStream(){}
	VideoReceiveStream::~VideoReceiveStream(){}

	bool VideoReceiveStream::init(libmedia_codec::VideoCodecType codec_type, int32_t width, int32_t height)
	{


		decoder_ = std::make_unique<libmedia_codec::H264Decoder>();
		
		decoder_->Configure(codec_type == libmedia_codec::kVideoCodecGb28181 ? libmedia_codec::kVideoCodecH264 : codec_type, width, height);
		decoder_->RegisterDecodeCompleteCallback(callback_);

		

		if (codec_type != libmedia_codec::kVideoCodecGb28181)
		{
			nal_parse_ = libmedia_codec::NalParseFactory::Create(
				codec_type == libmedia_codec::VideoCodecType::kVideoCodecH264 ?
				libmedia_codec::ENalParseType::ENalH264Prase :
				libmedia_codec::ENalParseType::ENalHEVCPrase);;
		}
		else
		{
			mpeg_decoder_ = std::make_unique<libmedia_transfer_protocol::libmpeg::MpegDecoder>();
			mpeg_decoder_->RegisterDecodeCompleteCallback(this);

			
		}
		return true;


		return false;
	}
	void VideoReceiveStream::OnVideoFrame(libmedia_codec::EncodedImage  image)
	{
		decoder_->Decode(image, true, 1);
	}
	void VideoReceiveStream::OnAudioFrame(rtc::Buffer frame)
	{
		//·ÖÀë adts 
#if 0

		uint8_t  pdata[7] = {
	0xff, /*1111 1111*/
	0xf1, /*1111 0001*/
	0x60, /*0110 0000*/
	0x40, /*0100 0000*/
	0x1f, /*0001 1111*/
	0x7f, /*011-1 1111*/
	0xfc  /*1111 1100*/
		};
		uint8_t *data = pdata;
		uint8_t p = data[3];
		int32_t   frame_length = (p & 0x03);
		frame_length <<= 8;
		int32_t p2 = data[4];
		frame_length |= p2;
		frame_length <<= 3;
		int32_t p3 = data[5];
		int32_t  ff = ((p3 >> 5));
		frame_length |= ff;

#endif // 
		uint8_t *data = frame.begin();
		uint8_t p = data[3];
		int32_t   frame_length = (p & 0x03);
		frame_length <<= 8;
		int32_t p2 = data[4];
		frame_length |= p2;
		frame_length <<= 3;
		int32_t p3 = data[5];
		int32_t  ff = ((p3 >> 5));
		frame_length |= ff;
		rtc::Buffer  new_aac_data(frame.begin() + 7, frame_length);
		audio_decder_->Decode(std::move(new_aac_data), 1);
	}
	void VideoReceiveStream::OnRtpPacket(const RtpPacketReceived & packet)
	{
		if (mpeg_decoder_)
		{
			mpeg_decoder_->parse(packet.payload().data(), packet.payload_size());
#if 0
			if (mpeg_decoder_->stream_len_ > 0 && mpeg_decoder_->read_byte_ == 0)
			{

			}libmedia_codec::EncodedImage encode_image;
			encode_image.SetEncodedData(
				libmedia_codec::EncodedImageBuffer::Create(
					mpeg_decoder_->h264_stream_,
					mpeg_decoder_->stream_len_
				));
			decoder_->Decode(encode_image, true, 1);
			mpeg_decoder_->stream_len_ = 0;
#endif // 
		}
		else if (nal_parse_)
		{
			nal_parse_->parse_packet(packet.payload().data(), packet.payload_size());
			if (packet.Marker())
			{
				libmedia_codec::EncodedImage encode_image;
				encode_image.SetEncodedData(
					libmedia_codec::EncodedImageBuffer::Create(
						nal_parse_->buffer_stream_,
						nal_parse_->buffer_index_
					));
				decoder_->Decode(encode_image, true, 1);
				nal_parse_->buffer_index_ = 0;
				//decoder_->Decode();
			}
		}
	}


//
//VideoReceiveStream::Decoder::Decoder(SdpVideoFormat video_format,
//                                     int payload_type)
//    : video_format(std::move(video_format)), payload_type(payload_type) {}
//VideoReceiveStream::Decoder::Decoder() : video_format("Unset") {}
//VideoReceiveStream::Decoder::Decoder(const Decoder&) = default;
//VideoReceiveStream::Decoder::~Decoder() = default;
//
//bool VideoReceiveStream::Decoder::operator==(const Decoder& other) const {
//  return payload_type == other.payload_type &&
//         video_format == other.video_format;
//}
//
//std::string VideoReceiveStream::Decoder::ToString() const {
//  char buf[1024];
//  rtc::SimpleStringBuilder ss(buf);
//  ss << "{payload_type: " << payload_type;
//  ss << ", payload_name: " << video_format.name;
//  ss << ", codec_params: {";
//  for (auto it = video_format.parameters.begin();
//       it != video_format.parameters.end(); ++it) {
//    if (it != video_format.parameters.begin()) {
//      ss << ", ";
//    }
//    ss << it->first << ": " << it->second;
//  }
//  ss << '}';
//  ss << '}';
//
//  return ss.str();
//}
//
//VideoReceiveStream::Stats::Stats() = default;
//VideoReceiveStream::Stats::~Stats() = default;
//
//std::string VideoReceiveStream::Stats::ToString(int64_t time_ms) const {
//  char buf[2048];
//  rtc::SimpleStringBuilder ss(buf);
//  ss << "VideoReceiveStream stats: " << time_ms << ", {ssrc: " << ssrc << ", ";
//  ss << "total_bps: " << total_bitrate_bps << ", ";
//  ss << "width: " << width << ", ";
//  ss << "height: " << height << ", ";
//  ss << "key: " << frame_counts.key_frames << ", ";
//  ss << "delta: " << frame_counts.delta_frames << ", ";
//  ss << "frames_dropped: " << frames_dropped << ", ";
//  ss << "network_fps: " << network_frame_rate << ", ";
//  ss << "decode_fps: " << decode_frame_rate << ", ";
//  ss << "render_fps: " << render_frame_rate << ", ";
//  ss << "decode_ms: " << decode_ms << ", ";
//  ss << "max_decode_ms: " << max_decode_ms << ", ";
//  ss << "first_frame_received_to_decoded_ms: "
//     << first_frame_received_to_decoded_ms << ", ";
//  ss << "cur_delay_ms: " << current_delay_ms << ", ";
//  ss << "targ_delay_ms: " << target_delay_ms << ", ";
//  ss << "jb_delay_ms: " << jitter_buffer_ms << ", ";
//  ss << "jb_cumulative_delay_seconds: " << jitter_buffer_delay_seconds << ", ";
//  ss << "jb_emitted_count: " << jitter_buffer_emitted_count << ", ";
//  ss << "min_playout_delay_ms: " << min_playout_delay_ms << ", ";
//  ss << "sync_offset_ms: " << sync_offset_ms << ", ";
//  ss << "cum_loss: " << rtp_stats.packets_lost << ", ";
//  ss << "nack: " << rtcp_packet_type_counts.nack_packets << ", ";
//  ss << "fir: " << rtcp_packet_type_counts.fir_packets << ", ";
//  ss << "pli: " << rtcp_packet_type_counts.pli_packets;
//  ss << '}';
//  return ss.str();
//}
//
//VideoReceiveStream::Config::Config(const Config&) = default;
//VideoReceiveStream::Config::Config(Config&&) = default;
//VideoReceiveStream::Config::Config(Transport* rtcp_send_transport,
//                                   VideoDecoderFactory* decoder_factory)
//    : decoder_factory(decoder_factory),
//      rtcp_send_transport(rtcp_send_transport) {}
//
//VideoReceiveStream::Config& VideoReceiveStream::Config::operator=(Config&&) =
//    default;
//VideoReceiveStream::Config::Config::~Config() = default;
//
//std::string VideoReceiveStream::Config::ToString() const {
//  char buf[4 * 1024];
//  rtc::SimpleStringBuilder ss(buf);
//  ss << "{decoders: [";
//  for (size_t i = 0; i < decoders.size(); ++i) {
//    ss << decoders[i].ToString();
//    if (i != decoders.size() - 1)
//      ss << ", ";
//  }
//  ss << ']';
//  ss << ", rtp: " << rtp.ToString();
//  ss << ", renderer: " << (renderer ? "(renderer)" : "nullptr");
//  ss << ", render_delay_ms: " << render_delay_ms;
//  if (!sync_group.empty())
//    ss << ", sync_group: " << sync_group;
//  ss << ", target_delay_ms: " << target_delay_ms;
//  ss << '}';
//
//  return ss.str();
//}
//
//VideoReceiveStream::Config::Rtp::Rtp() = default;
//VideoReceiveStream::Config::Rtp::Rtp(const Rtp&) = default;
//VideoReceiveStream::Config::Rtp::~Rtp() = default;
//
//std::string VideoReceiveStream::Config::Rtp::ToString() const {
//  char buf[2 * 1024];
//  rtc::SimpleStringBuilder ss(buf);
//  ss << "{remote_ssrc: " << remote_ssrc;
//  ss << ", local_ssrc: " << local_ssrc;
//  ss << ", rtcp_mode: "
//     << (rtcp_mode == RtcpMode::kCompound ? "RtcpMode::kCompound"
//                                          : "RtcpMode::kReducedSize");
//  ss << ", rtcp_xr: ";
//  ss << "{receiver_reference_time_report: "
//     << (rtcp_xr.receiver_reference_time_report ? "on" : "off");
//  ss << '}';
//  ss << ", transport_cc: " << (transport_cc ? "on" : "off");
//  ss << ", lntf: {enabled: " << (lntf.enabled ? "true" : "false") << '}';
//  ss << ", nack: {rtp_history_ms: " << nack.rtp_history_ms << '}';
//  ss << ", ulpfec_payload_type: " << ulpfec_payload_type;
//  ss << ", red_type: " << red_payload_type;
//  ss << ", rtx_ssrc: " << rtx_ssrc;
//  ss << ", rtx_payload_types: {";
//  for (auto& kv : rtx_associated_payload_types) {
//    ss << kv.first << " (pt) -> " << kv.second << " (apt), ";
//  }
//  ss << '}';
//  ss << ", raw_payload_types: {";
//  for (const auto& pt : raw_payload_types) {
//    ss << pt << ", ";
//  }
//  ss << '}';
//  ss << ", extensions: [";
//  for (size_t i = 0; i < extensions.size(); ++i) {
//    ss << extensions[i].ToString();
//    if (i != extensions.size() - 1)
//      ss << ", ";
//  }
//  ss << ']';
//  ss << '}';
//  return ss.str();
//}

}  // namespace webrtc

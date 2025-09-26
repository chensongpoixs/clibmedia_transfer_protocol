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
				   date:  2025-09-21



 ******************************************************************************/

#ifndef _C_CODEC_H_
#define _C_CODEC_H_

#include <string>
#include <map>
#include <vector>
#include "libmedia_transfer_protocol/media_constants.h"
#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/rtp_parameters.h"
#include "libmedia_codec/video_codecs/sdp_video_format.h"
namespace libmedia_transfer_protocol
{
 
	struct FeedbackParam { 
	public:
		FeedbackParam() = default;
		FeedbackParam(const std::string& id, const std::string& param)
			: id_(id), param_(param) {}
		explicit FeedbackParam(const std::string& id)
			: id_(id), param_(kParamValueEmpty) {}

		bool operator==(const FeedbackParam& other) const;

		const std::string& id() const { return id_; }
		const std::string& param() const { return param_; }
 
		std::string id_   ;     // e.g. "nack", "ccm"
		std::string param_ = libmedia_transfer_protocol::kParamValueEmpty;  // e.g. "", "rpsi", "fir"
	};  

	class FeedbackParams {
	public:
		FeedbackParams();
		~FeedbackParams();
		bool operator==(const FeedbackParams& other) const;

		bool Has(const FeedbackParam& param) const;
		void Add(const FeedbackParam& param);

		void Intersect(const FeedbackParams& from);

		const std::vector<FeedbackParam>& params() const { return params_; }

	
		bool HasDuplicateEntries() const;

		std::vector<FeedbackParam> params_;
	};
	struct   Codec {
		int id;
		std::string name;
		int clockrate;

		virtual  size_t  GetChannel() const   { return 0; }
		// Non key-value parameters such as the telephone-event "0‐15" are
		// represented using an empty string as key, i.e. {"": "0-15"}.
		// fmtp
		std::map<std::string, std::string> params;
		// rtcp-fb
		 FeedbackParams feedback_params;


		virtual ~Codec();

		// Indicates if this codec is compatible with the specified codec.
		bool Matches(const Codec& codec) const;
		bool MatchesCapability(const  RtpCodecCapability& capability) const;

		// Find the parameter for `name` and write the value to `out`.
		bool GetParam(const std::string& name, std::string* out) const;
		bool GetParam(const std::string& name, int* out) const;

		void SetParam(const std::string& name, const std::string& value);
		void SetParam(const std::string& name, int value);

		// It is safe to input a non-existent parameter.
		// Returns true if the parameter existed, false if it did not exist.
		bool RemoveParam(const std::string& name);

		bool HasFeedbackParam(const FeedbackParam& param) const;
		void AddFeedbackParam(const FeedbackParam& param);

		// Filter `this` feedbacks params such that only those shared by both `this`
		// and `other` are kept.
		void IntersectFeedbackParams(const Codec& other);

		virtual  RtpCodecParameters ToCodecParameters() const;

		Codec& operator=(const Codec& c);
		Codec& operator=(Codec&& c);

		bool operator==(const Codec& c) const;

		bool operator!=(const Codec& c) const { return !(*this == c); }

	protected:
		// A Codec can't be created without a subclass.
		// Creates a codec with the given parameters.
		Codec(int id, const std::string& name, int clockrate);
		// Creates an empty codec.
		Codec();
		Codec(const Codec& c);
		Codec(Codec&& c);
	};



	struct AudioCodec : public Codec {
		int bitrate;
		size_t channels;
		virtual  size_t  GetChannel() const override { return channels; }
		 
		std::string ToString() const; 
	};

	struct   VideoCodec : public Codec {
		absl::optional<std::string> packetization;

		 

		// Creates a codec with the given parameters.
		VideoCodec(int id, const std::string& name);
		// Creates a codec with the given name and empty id.
		explicit VideoCodec(const std::string& name);
		// Creates an empty codec.
		VideoCodec();
		VideoCodec(const VideoCodec& c);
		explicit VideoCodec(const libmedia_codec::SdpVideoFormat& c);
		VideoCodec(VideoCodec&& c);
		~VideoCodec() override = default;

		// Indicates if this video codec is the same as the other video codec, e.g. if
		// they are both VP8 or VP9, or if they are both H264 with the same H264
		// profile. H264 levels however are not compared.
		bool Matches(const VideoCodec& codec) const;

		std::string ToString() const;

		 RtpCodecParameters ToCodecParameters() const override;

		VideoCodec& operator=(const VideoCodec& c);
		VideoCodec& operator=(VideoCodec&& c);

		bool operator==(const VideoCodec& c) const;

		bool operator!=(const VideoCodec& c) const { return !(*this == c); }

		// Return packetization which both `local_codec` and `remote_codec` support.
		static absl::optional<std::string> IntersectPacketization(
			const VideoCodec& local_codec,
			const VideoCodec& remote_codec);

		static VideoCodec CreateRtxCodec(int rtx_payload_type,
			int associated_payload_type);

		

		enum CodecType {
			CODEC_VIDEO,
			CODEC_RED,
			CODEC_ULPFEC,
			CODEC_FLEXFEC,
			CODEC_RTX,
		}; 
		CodecType GetCodecType() const;
		bool ValidateCodecFormat() const;

 
		void SetDefaultParameters();
	};
	// Get the codec setting associated with `payload_type`. If there
// is no codec associated with that payload type it returns nullptr.
	template <class Codec>
	const Codec* FindCodecById(const std::vector<Codec>& codecs, int payload_type) {
		for (const auto& codec : codecs) {
			if (codec.id == payload_type)
				return &codec;
		}
		return nullptr;
	}

	bool HasLntf(const Codec& codec);
	bool HasNack(const Codec& codec);
	bool HasRemb(const Codec& codec);
	bool HasRrtr(const Codec& codec);
	bool HasTransportCc(const Codec& codec);
	// Returns the first codec in `supported_codecs` that matches `codec`, or
	// nullptr if no codec matches.
	const VideoCodec* FindMatchingCodec(
		const std::vector<VideoCodec>& supported_codecs,
		const VideoCodec& codec);

	RTC_EXPORT void AddH264ConstrainedBaselineProfileToSupportedFormats(
		std::vector<libmedia_codec::SdpVideoFormat>* supported_formats);

}

#endif // _C_CODEC_H_
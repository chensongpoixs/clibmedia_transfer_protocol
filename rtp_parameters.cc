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

#include "libmedia_transfer_protocol/rtp_parameters.h"

#include <algorithm>
#include <string>
#include <utility>

#include "api/array_view.h"
#include "rtc_base/strings/string_builder.h"

namespace libmedia_transfer_protocol {

 
const double kDefaultBitratePriority = 1.0;

RtpExtension::RtpExtension() = default;
RtpExtension::RtpExtension(absl::string_view uri, int id) : uri(uri), id(id) {}
RtpExtension::RtpExtension(absl::string_view uri, int id, bool encrypt)
	: uri(uri), id(id), encrypt(encrypt) {}
RtpExtension::~RtpExtension() = default;
std::string RtpExtension::ToString() const {
  char buf[256];
  rtc::SimpleStringBuilder sb(buf);
  sb << "{uri: " << uri;
  sb << ", id: " << id;
  if (encrypt) {
    sb << ", encrypt";
  }
  sb << '}';
  return sb.str();
}

constexpr char RtpExtension::kEncryptHeaderExtensionsUri[];
constexpr char RtpExtension::kAudioLevelUri[];
constexpr char RtpExtension::kTimestampOffsetUri[];
constexpr char RtpExtension::kAbsSendTimeUri[];
constexpr char RtpExtension::kAbsoluteCaptureTimeUri[];
constexpr char RtpExtension::kVideoRotationUri[];
constexpr char RtpExtension::kVideoContentTypeUri[];
constexpr char RtpExtension::kVideoTimingUri[];
constexpr char RtpExtension::kGenericFrameDescriptorUri00[];
constexpr char RtpExtension::kDependencyDescriptorUri[];
constexpr char RtpExtension::kVideoLayersAllocationUri[];
constexpr char RtpExtension::kTransportSequenceNumberUri[];
constexpr char RtpExtension::kTransportSequenceNumberV2Uri[];
constexpr char RtpExtension::kPlayoutDelayUri[];
constexpr char RtpExtension::kColorSpaceUri[];
constexpr char RtpExtension::kMidUri[];
constexpr char RtpExtension::kRidUri[];
constexpr char RtpExtension::kRepairedRidUri[];
constexpr char RtpExtension::kVideoFrameTrackingIdUri[];
constexpr char RtpExtension::kCsrcAudioLevelsUri[];

constexpr int RtpExtension::kMinId;
constexpr int RtpExtension::kMaxId;
constexpr int RtpExtension::kMaxValueSize;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxId;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxValueSize;
 


RtcpParameters::RtcpParameters() = default;
RtcpParameters::RtcpParameters(const RtcpParameters& rhs) = default;
RtcpParameters::~RtcpParameters() = default;


bool RtpExtension::IsSupportedForAudio(absl::string_view uri) {
	return uri ==  RtpExtension::kAudioLevelUri ||
		uri ==  RtpExtension::kAbsSendTimeUri ||
		uri ==  RtpExtension::kAbsoluteCaptureTimeUri ||
		uri ==  RtpExtension::kTransportSequenceNumberUri ||
		uri ==  RtpExtension::kTransportSequenceNumberV2Uri ||
		uri ==  RtpExtension::kMidUri ||
		uri ==  RtpExtension::kRidUri ||
		uri ==  RtpExtension::kRepairedRidUri;
}

bool RtpExtension::IsSupportedForVideo(absl::string_view uri) {
	return uri ==  RtpExtension::kTimestampOffsetUri ||
		uri ==  RtpExtension::kAbsSendTimeUri ||
		uri ==  RtpExtension::kAbsoluteCaptureTimeUri ||
		uri ==  RtpExtension::kVideoRotationUri ||
		uri ==  RtpExtension::kTransportSequenceNumberUri ||
		uri ==  RtpExtension::kTransportSequenceNumberV2Uri ||
		uri ==  RtpExtension::kPlayoutDelayUri ||
		uri ==  RtpExtension::kVideoContentTypeUri ||
		uri ==  RtpExtension::kVideoTimingUri ||
		uri ==  RtpExtension::kMidUri ||
		uri ==  RtpExtension::kGenericFrameDescriptorUri00 ||
		uri ==  RtpExtension::kDependencyDescriptorUri ||
		uri ==  RtpExtension::kColorSpaceUri ||
		uri ==  RtpExtension::kRidUri ||
		uri ==  RtpExtension::kRepairedRidUri ||
		uri ==  RtpExtension::kVideoLayersAllocationUri ||
		uri ==  RtpExtension::kVideoFrameTrackingIdUri;
}

bool RtpExtension::IsEncryptionSupported(absl::string_view uri) {
	return
#if defined(ENABLE_EXTERNAL_AUTH)
		// TODO(jbauch): Figure out a way to always allow "kAbsSendTimeUri"
		// here and filter out later if external auth is really used in
		// srtpfilter. External auth is used by Chromium and replaces the
		// extension header value of "kAbsSendTimeUri", so it must not be
		// encrypted (which can't be done by Chromium).
		uri != webrtc::RtpExtension::kAbsSendTimeUri &&
#endif
		uri !=  RtpExtension::kEncryptHeaderExtensionsUri;
}

// Returns whether a header extension with the given URI exists.
// Note: This does not differentiate between encrypted and non-encrypted
// extensions, so use with care!
static bool HeaderExtensionWithUriExists(
	const std::vector<RtpExtension>& extensions,
	absl::string_view uri) {
	for (const auto& extension : extensions) {
		if (extension.uri == uri) {
			return true;
		}
	}
	return false;
}

const RtpExtension* RtpExtension::FindHeaderExtensionByUri(
	const std::vector<RtpExtension>& extensions,
	absl::string_view uri,
	Filter filter) {
	const  RtpExtension* fallback_extension = nullptr;
	for (const auto& extension : extensions) {
		if (extension.uri != uri) {
			continue;
		}

		switch (filter) {
		case kDiscardEncryptedExtension:
			// We only accept an unencrypted extension.
			if (!extension.encrypt) {
				return &extension;
			}
			break;

		case kPreferEncryptedExtension:
			// We prefer an encrypted extension but we can fall back to an
			// unencrypted extension.
			if (extension.encrypt) {
				return &extension;
			}
			else {
				fallback_extension = &extension;
			}
			break;

		case kRequireEncryptedExtension:
			// We only accept an encrypted extension.
			if (extension.encrypt) {
				return &extension;
			}
			break;
		}
	}

	// Returning fallback extension (if any)
	return fallback_extension;
}

const RtpExtension* RtpExtension::FindHeaderExtensionByUri(
	const std::vector<RtpExtension>& extensions,
	absl::string_view uri) {
	return FindHeaderExtensionByUri(extensions, uri, kPreferEncryptedExtension);
}

const RtpExtension* RtpExtension::FindHeaderExtensionByUriAndEncryption(
	const std::vector<RtpExtension>& extensions,
	absl::string_view uri,
	bool encrypt) {
	for (const auto& extension : extensions) {
		if (extension.uri == uri && extension.encrypt == encrypt) {
			return &extension;
		}
	}
	return nullptr;
}

const std::vector<RtpExtension> RtpExtension::DeduplicateHeaderExtensions(
	const std::vector<RtpExtension>& extensions,
	Filter filter) {
	std::vector<RtpExtension> filtered;

	// If we do not discard encrypted extensions, add them first
	if (filter != kDiscardEncryptedExtension) {
		for (const auto& extension : extensions) {
			if (!extension.encrypt) {
				continue;
			}
			if (!HeaderExtensionWithUriExists(filtered, extension.uri)) {
				filtered.push_back(extension);
			}
		}
	}

	// If we do not require encrypted extensions, add missing, non-encrypted
	// extensions.
	if (filter != kRequireEncryptedExtension) {
		for (const auto& extension : extensions) {
			if (extension.encrypt) {
				continue;
			}
			if (!HeaderExtensionWithUriExists(filtered, extension.uri)) {
				filtered.push_back(extension);
			}
		}
	}

	return filtered;
}



RtpHeaderExtensionCapability::RtpHeaderExtensionCapability() = default;
RtpHeaderExtensionCapability::RtpHeaderExtensionCapability(
	absl::string_view uri)
	: uri(uri) {}
RtpHeaderExtensionCapability::RtpHeaderExtensionCapability(
	absl::string_view uri,
	int preferred_id)
	: uri(uri), preferred_id(preferred_id) {}
RtpHeaderExtensionCapability::RtpHeaderExtensionCapability(
	absl::string_view uri,
	int preferred_id,
	RtpTransceiverDirection direction)
	: uri(uri), preferred_id(preferred_id), direction(direction) {}
RtpHeaderExtensionCapability::~RtpHeaderExtensionCapability() = default;


RtpCodecParameters::RtpCodecParameters() = default;
RtpCodecParameters::RtpCodecParameters(const RtpCodecParameters& rhs) = default;
RtpCodecParameters::~RtpCodecParameters() = default;

RtcpFeedback::RtcpFeedback() = default;
RtcpFeedback::RtcpFeedback(RtcpFeedbackType type) : type(type) {}
RtcpFeedback::RtcpFeedback(RtcpFeedbackType type,
	RtcpFeedbackMessageType message_type)
	: type(type), message_type(message_type) {}
RtcpFeedback::RtcpFeedback(const RtcpFeedback& rhs) = default;
RtcpFeedback::~RtcpFeedback() = default;
}  // namespace webrtc

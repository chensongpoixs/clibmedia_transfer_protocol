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


#include "libmedia_transfer_protocol/media_stream_interface.h"
#include "libmedia_transfer_protocol/media_types.h"

namespace libmedia_transfer_protocol {

const char* const MediaStreamTrackInterface::kVideoKind =
libmedia_transfer_protocol::kMediaTypeVideo;
const char* const MediaStreamTrackInterface::kAudioKind =
libmedia_transfer_protocol::kMediaTypeAudio;

VideoTrackInterface::ContentHint VideoTrackInterface::content_hint() const {
  return ContentHint::kNone;
}

bool AudioTrackInterface::GetSignalLevel(int* level) {
  return false;
}

rtc::scoped_refptr<AudioProcessorInterface>
AudioTrackInterface::GetAudioProcessor() {
  return nullptr;
}

const cricket::AudioOptions AudioSourceInterface::options() const {
  return {};
}

}  // namespace webrtc

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
				   date:  2025-10-06



 ******************************************************************************/



#ifndef _C_MODULES_VIDEO_CODING_RTP_VP9_REF_FINDER_H_
#define _C_MODULES_VIDEO_CODING_RTP_VP9_REF_FINDER_H_

#include <deque>
#include <map>
#include <memory>
#include <set>

#include "absl/container/inlined_vector.h"
#include "libmedia_codec/frame_object.h"
#include "libmedia_transfer_protocol/rtp_frame_reference_finder.h"
#include "rtc_base/numerics/sequence_number_util.h"

namespace libmedia_transfer_protocol {

class RtpVp9RefFinder {
 public:
  RtpVp9RefFinder() = default;

  RtpFrameReferenceFinder::ReturnVector ManageFrame(
      std::unique_ptr<libmedia_codec::RtpFrameObject> frame);
  void ClearTo(uint16_t seq_num);

 private:
  static constexpr int kFrameIdLength = 1 << 15;
  static constexpr int kMaxGofSaved = 50;
  static constexpr int kMaxLayerInfo = 50;
  static constexpr int kMaxNotYetReceivedFrames = 100;
  static constexpr int kMaxStashedFrames = 100;
  static constexpr int kMaxTemporalLayers = 5;

  enum FrameDecision { kStash, kHandOff, kDrop };

  struct GofInfo {
    GofInfo(webrtc::GofInfoVP9* gof, uint16_t last_picture_id)
        : gof(gof), last_picture_id(last_picture_id) {}
    webrtc::GofInfoVP9* gof;
    uint16_t last_picture_id;
  };

  FrameDecision ManageFrameInternal(libmedia_codec::RtpFrameObject* frame);
  void RetryStashedFrames(RtpFrameReferenceFinder::ReturnVector& res);

  bool MissingRequiredFrameVp9(uint16_t picture_id, const GofInfo& info);

  void FrameReceivedVp9(uint16_t picture_id, GofInfo* info);
  bool UpSwitchInIntervalVp9(uint16_t picture_id,
                             uint8_t temporal_idx,
                             uint16_t pid_ref);

  void FlattenFrameIdAndRefs(libmedia_codec::RtpFrameObject* frame, bool inter_layer_predicted);

  // Save the last picture id in order to detect when there is a gap in frames
  // that have not yet been fully received.
  int last_picture_id_ = -1;

  // Frames that have been fully received but didn't have all the information
  // needed to determine their references.
  std::deque<std::unique_ptr<libmedia_codec::RtpFrameObject>> stashed_frames_;

  // Where the current scalability structure is in the
  // `scalability_structures_` array.
  uint8_t current_ss_idx_ = 0;

  // Holds received scalability structures.
  std::array<webrtc::GofInfoVP9, kMaxGofSaved> scalability_structures_;

  // Holds the the Gof information for a given unwrapped TL0 picture index.
  std::map<int64_t, GofInfo> gof_info_;

  // Keep track of which picture id and which temporal layer that had the
  // up switch flag set.
  std::map<uint16_t, uint8_t, webrtc::DescendingSeqNumComp<uint16_t, kFrameIdLength>>
      up_switch_;

  // For every temporal layer, keep a set of which frames that are missing.
  std::array<std::set<uint16_t, webrtc::DescendingSeqNumComp<uint16_t, kFrameIdLength>>,
             kMaxTemporalLayers>
      missing_frames_for_layer_;

  // Unwrapper used to unwrap VP8/VP9 streams which have their picture id
  // specified.
  webrtc::SeqNumUnwrapper<uint16_t, kFrameIdLength> unwrapper_;

  webrtc::SeqNumUnwrapper<uint8_t> tl0_unwrapper_;
};

}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_RTP_VP9_REF_FINDER_H_

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
// This file contains structures for describing SSRCs from a media source such
// as a MediaStreamTrack when it is sent across an RTP session. Multiple media
// sources may be sent across the same RTP session, each of them will be
// described by one StreamParams object
// SsrcGroup is used to describe the relationship between the SSRCs that
// are used for this media source.
// E.x: Consider a source that is sent as 3 simulcast streams
// Let the simulcast elements have SSRC 10, 20, 30.
// Let each simulcast element use FEC and let the protection packets have
// SSRC 11,21,31.
// To describe this 4 SsrcGroups are needed,
// StreamParams would then contain ssrc = {10,11,20,21,30,31} and
// ssrc_groups = {{SIM,{10,20,30}, {FEC,{10,11}, {FEC, {20,21}, {FEC {30,31}}}
// Please see RFC 5576.
// A spec-compliant way to achieve this is to use RIDs and Simulcast attribute
// instead of the ssrc-group. In this method, the StreamParam object will
// have multiple RidDescriptions, each corresponding to a simulcast layer
// and the media section will have a simulcast attribute that indicates
// that these layers are for the same source. This also removes the extra
// lines for redundancy streams, as the same RIDs appear in the redundancy
// packets.
// Note: in the spec compliant simulcast scenario, some of the RIDs might be
// alternatives for one another (such as different encodings for same data).
// In the context of the StreamParams class, the notion of alternatives does
// not exist and all the RIDs will describe different layers of the same source.
// When the StreamParams class is used to configure the media engine, simulcast
// considerations will be used to remove the alternative layers outside of this
// class.
// As an example, let the simulcast layers have RID 10, 20, 30.
// StreamParams would contain rid = { 10, 20, 30 }.
// MediaSection would contain SimulcastDescription specifying these rids.
// a=simulcast:send 10;20;30 (or a=simulcast:send 10,20;30 or similar).
// See https://tools.ietf.org/html/draft-ietf-mmusic-sdp-simulcast-13
// and https://tools.ietf.org/html/draft-ietf-mmusic-rid-15.

#ifndef _C_MEDIA_BASE_STREAM_PARAMS_H_
#define _C_MEDIA_BASE_STREAM_PARAMS_H_

#include <stddef.h>

#include <cstdint>
#include <string>
#include <vector>

#include "absl/algorithm/container.h"
//#include "media/base/rid_description.h"
#include "libmedia_transfer_protocol/rid_description.h"
#include "rtc_base/constructor_magic.h"
#include "rtc_base/unique_id_generator.h"

namespace libmedia_transfer_protocol {

extern const char kFecSsrcGroupSemantics[];
extern const char kFecFrSsrcGroupSemantics[];
extern const char kFidSsrcGroupSemantics[];
extern const char kSimSsrcGroupSemantics[];

struct SsrcGroup {
   
  std::string ToString() const;

  std::string semantics;        // e.g FIX, FEC, SIM.
  std::vector<uint32_t> ssrcs;  // SSRCs of this type.
};

// StreamParams is used to represent a sender/track in a SessionDescription.
// In Plan B, this means that multiple StreamParams can exist within one
// MediaContentDescription, while in UnifiedPlan this means that there is one
// StreamParams per MediaContentDescription.
struct StreamParams {
    
  uint32_t first_ssrc() const {
    if (ssrcs.empty()) {
      return 0;
    }

    return ssrcs[0];
  }
   

   

  std::string ToString() const;
 
  std::string groupid;
  // A unique identifier of the StreamParams object. When the SDP is created,
  // this comes from the track ID of the sender that the StreamParams object
  // is associated with.
  std::string id;
  // There may be no SSRCs stored in unsignaled case when stream_ids are
  // signaled with a=msid lines.
  std::vector<uint32_t> ssrcs;         // All SSRCs for this source
  std::vector<SsrcGroup> ssrc_groups;  // e.g. FID, FEC, SIM
  std::string cname;                   // RTCP CNAME
  
  std::vector<std::string> stream_ids_;

  std::vector<RidDescription> rids_;
};

// A Stream can be selected by either groupid+id or ssrc.
struct StreamSelector {
   
  uint32_t ssrc;
  std::string groupid;
  std::string streamid;
};

typedef std::vector<StreamParams> StreamParamsVec;

template <class Condition>
const StreamParams* GetStream(const StreamParamsVec& streams,
                              Condition condition) {
  //auto found = absl::c_find_if(streams, condition);
	return nullptr;
  //return found == streams.end() ? nullptr : &(*found);
}
 
}  // namespace cricket

#endif  // MEDIA_BASE_STREAM_PARAMS_H_

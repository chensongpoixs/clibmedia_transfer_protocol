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

#include "libmedia_transfer_protocol/stream_params.h"

#include <stdint.h>

#include <list>

#include "absl/algorithm/container.h"
#include "api/array_view.h"
#include "rtc_base/strings/string_builder.h"

namespace libmedia_transfer_protocol {
namespace {

void AppendSsrcs(rtc::ArrayView<const uint32_t> ssrcs,
                 rtc::SimpleStringBuilder* sb) {
  *sb << "ssrcs:[";
  const char* delimiter = "";
  for (uint32_t ssrc : ssrcs) {
    *sb << delimiter << ssrc;
    delimiter = ",";
  }
  *sb << "]";
}

void AppendSsrcGroups(rtc::ArrayView<const SsrcGroup> ssrc_groups,
                      rtc::SimpleStringBuilder* sb) {
  *sb << "ssrc_groups:";
  const char* delimiter = "";
  for (const SsrcGroup& ssrc_group : ssrc_groups) {
    *sb << delimiter << ssrc_group.ToString();
    delimiter = ",";
  }
}

void AppendStreamIds(rtc::ArrayView<const std::string> stream_ids,
                     rtc::SimpleStringBuilder* sb) {
  *sb << "stream_ids:";
  const char* delimiter = "";
  for (const std::string& stream_id : stream_ids) {
    *sb << delimiter << stream_id;
    delimiter = ",";
  }
}

void AppendRids(rtc::ArrayView<const RidDescription> rids,
                rtc::SimpleStringBuilder* sb) {
  *sb << "rids:[";
  const char* delimiter = "";
  for (const RidDescription& rid : rids) {
    *sb << delimiter << rid.rid;
    delimiter = ",";
  }
  *sb << "]";
}

}  // namespace

const char kFecSsrcGroupSemantics[] = "FEC";
const char kFecFrSsrcGroupSemantics[] = "FEC-FR";
const char kFidSsrcGroupSemantics[] = "FID";
const char kSimSsrcGroupSemantics[] = "SIM";

bool GetStream(const StreamParamsVec& streams,
               const StreamSelector& selector,
               StreamParams* stream_out) {
  const StreamParams* found = GetStream(streams, selector);
  if (found && stream_out)
    *stream_out = *found;
  return found != nullptr;
}


std::string SsrcGroup::ToString() const {
  char buf[1024];
  rtc::SimpleStringBuilder sb(buf);
  sb << "{";
  sb << "semantics:" << semantics << ";";
  AppendSsrcs(ssrcs, &sb);
  sb << "}";
  return sb.str();
}

  

std::string StreamParams::ToString() const {
  char buf[2 * 1024];
  rtc::SimpleStringBuilder sb(buf);
  sb << "{";
  if (!groupid.empty()) {
    sb << "groupid:" << groupid << ";";
  }
  if (!id.empty()) {
    sb << "id:" << id << ";";
  }
  AppendSsrcs(ssrcs, &sb);
  sb << ";";
  AppendSsrcGroups(ssrc_groups, &sb);
  sb << ";";
  if (!cname.empty()) {
    sb << "cname:" << cname << ";";
  }
  AppendStreamIds(stream_ids_, &sb);
  sb << ";";
  if (!rids_.empty()) {
    AppendRids(rids_, &sb);
    sb << ";";
  }
  sb << "}";
  return sb.str();
}
 

}  // namespace cricket

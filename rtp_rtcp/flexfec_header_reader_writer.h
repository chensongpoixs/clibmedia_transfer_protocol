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
				   date:  2025-09-24



 ******************************************************************************/



#ifndef _C_MODULES_RTP_RTCP_SOURCE_FLEXFEC_HEADER_READER_WRITER_H_
#define _C_MODULES_RTP_RTCP_SOURCE_FLEXFEC_HEADER_READER_WRITER_H_

#include <stddef.h>
#include <stdint.h>

#include "libmedia_transfer_protocol/rtp_rtcp/forward_error_correction.h"

namespace libmedia_transfer_protocol {

// FlexFEC header, minimum 20 bytes.
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  0 |R|F|P|X|  CC   |M| PT recovery |        length recovery        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 |                          TS recovery                          |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |   SSRCCount   |                    reserved                   |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// 12 |                             SSRC_i                            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |           SN base_i           |k|          Mask [0-14]        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 20 |k|                   Mask [15-45] (optional)                   |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 24 |k|                                                             |
//    +-+                   Mask [46-108] (optional)                  |
// 28 |                                                               |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//    :                     ... next in SSRC_i ...                    :
//
//
// FlexFEC header in 'inflexible' mode (F = 1), 20 bytes.
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  0 |0|1|P|X|  CC   |M| PT recovery |        length recovery        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 |                          TS recovery                          |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |   SSRCCount   |                    reserved                   |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |                             SSRC_i                            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |           SN base_i           |  M (columns)  |    N (rows)   |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class FlexfecHeaderReader : public FecHeaderReader {
 public:
  FlexfecHeaderReader();
  ~FlexfecHeaderReader() override;

  bool ReadFecHeader(
      ForwardErrorCorrection::ReceivedFecPacket* fec_packet) const override;
};

class FlexfecHeaderWriter : public FecHeaderWriter {
 public:
  FlexfecHeaderWriter();
  ~FlexfecHeaderWriter() override;

  size_t MinPacketMaskSize(const uint8_t* packet_mask,
                           size_t packet_mask_size) const override;

  size_t FecHeaderSize(size_t packet_mask_row_size) const override;

  void FinalizeFecHeader(
      uint32_t media_ssrc,
      uint16_t seq_num_base,
      const uint8_t* packet_mask,
      size_t packet_mask_size,
      ForwardErrorCorrection::Packet* fec_packet) const override;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_SOURCE_FLEXFEC_HEADER_READER_WRITER_H_

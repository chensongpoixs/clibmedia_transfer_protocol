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
				   date:  2025-10-14



 ******************************************************************************/


#ifndef _C_STUN_H_
#define _C_STUN_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <string>
#include "rtc_base/buffer.h"

namespace libmedia_transfer_protocol {
	namespace librtc {


		const uint32_t kStunMagicCookie = 0x2112A442;
		enum StunMessageType
		{
			kStunMsgUnknow = 0x0000,
			kStunMsgBindingRequest = 0x0001,
			kStunMsgBindingResponse = 0x0101,
			kStunMsgBindingErrorResponse = 0x0111,
			kStunMsgSharedSecretRequest = 0x0002,
			kStunMsgSharedSecretResponse = 0x0102,
			kStunMsgSharedSecretErrorResponse = 0x0112,
		};

		enum StunAttributeType
		{
			// https://tools.ietf.org/html/rfc3489#section-11.2
			kStunAttrMappedAddress = 0x0001,
			kStunAttrResponseAddress = 0x0002,
			kStunAttrChangeRequest = 0x0003,
			kStunAttrSourceAddress = 0x0004,
			kStunAttrChangedAddress = 0x0005,
			kStunAttrUsername = 0x0006,
			kStunAttrPassword = 0x0007,
			kStunAttrMessageIntegrity = 0x0008,
			kStunAttrErrorCode = 0x0009,
			kStunAttrUnknownAttributes = 0x000A,
			kStunAttrReflectedFrom = 0x000B,
			// https://tools.ietf.org/html/rfc5389#section-18.2
			kStunAttrRealm = 0x0014,
			kStunAttrNonce = 0x0015,
			kStunAttrXorMappedAddress = 0x0020,
			kStunAttrSoftware = 0x8022,
			kStunAttrAlternateServer = 0x8023,
			kStunAttrFingerprint = 0x8028,
			kStunAttrPriority = 0x0024,
			kStunAttrUseCandidate = 0x0025,
			kStunAttrIceControlled = 0x8029,
			kStunAttrIceControlling = 0x802A,
		};
		class Stun
		{
		public:
			Stun() = default;
			~Stun() = default;
		public:
			bool Decode(const uint8_t* data, uint32_t size);
			rtc::Buffer Encode();
			std::string LocalUFrag();
			void SetPassword(const std::string &pwd);
			void SetMappedAddr(uint32_t addr);
			void SetMappedPort(uint16_t port);
			void SetMessageType(StunMessageType type);
			size_t CalcHmac(char *buf, const char *data, size_t bytes);
		private:
			StunMessageType type_{ kStunMsgUnknow };
			int32_t stun_length_{ 0 };
			std::string transcation_id_;
			std::string user_name_{""};
			std::string password_{ "" };
			uint32_t mapped_addr_{ 0 };
			uint16_t mapped_port_{ 0 };
		};
	}
}


#endif//
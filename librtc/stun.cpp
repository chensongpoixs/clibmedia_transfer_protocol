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




#include "libmedia_transfer_protocol/librtc/stun.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
#include "rtc_base/logging.h"
#include "rtc_base/buffer.h"
#include "rtc_base/crc32.h"
namespace libmedia_transfer_protocol {
	namespace librtc {

		bool Stun::Decode(const uint8_t* data, uint32_t size)
		{
			type_ = (StunMessageType)ByteReader<uint16_t>::ReadBigEndian(data);
			data += 2;
			stun_length_ = ByteReader<uint16_t>::ReadBigEndian(data);
			data += 2;
			auto magic = ByteReader<uint32_t>::ReadBigEndian(data);
			if (magic != kStunMagicCookie)
			{
				LIBRTC_LOG(LS_WARNING) << "stun magic:" << magic << " not equal kStunMagicCookie:" << kStunMagicCookie;
				return false;
			}
			data += 4;

			transcation_id_.assign((char *)data, 12);
			data += 12;

			LIBRTC_LOG(LS_WARNING) << "stun type:" << type_
				<< " length:" << stun_length_
				<< " transcation_id:" << transcation_id_;
			size -= 20;

			while (size >= 4)
			{
				uint16_t attr_type = ByteReader<uint16_t>::ReadBigEndian(data);
				data += 2;
				uint16_t attr_len = ByteReader<uint16_t>::ReadBigEndian(data);
				data += 2;
				size -= 4;

				uint16_t padding_len = (4 - attr_len % 4) % 4;
				if (size < padding_len + attr_len)
				{
					LIBRTC_LOG(LS_WARNING) << "stun attr len:" << attr_len
						<< " padding:" << padding_len
						<< " size:" << size;
					return false;
				}

				switch (attr_type)
				{
				case kStunAttrUsername:
				{
					user_name_.assign((char *)data, attr_len);
					LIBRTC_LOG(LS_INFO) << "stun user name:" << user_name_;
					break;
				}
				case kStunAttrPassword:
				{
					password_.assign((char *)data, attr_len);
					LIBRTC_LOG(LS_INFO) << "stun passwd:" << password_;
					break;
				}
				}

				data += (attr_len + padding_len);
				size -= (attr_len + padding_len);
			}
			return true;
		}

		rtc::Buffer Stun::Encode()
		{
			rtc::Buffer packet(512);
			int32_t  stun_length = 0;
			//PacketPtr packet = Packet::NewPacket(512);
			uint8_t *data = packet.begin();//packet->Data();

			/*data +=*/ ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)type_);
			//data += 2;
			stun_length += 2;
			/*data +=*/ ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)0);
			stun_length += 2;
			//data += 2;
			/*data +=*/ ByteWriter<uint32_t>::WriteBigEndian(data + stun_length, (uint32_t)kStunMagicCookie);
			stun_length += 4;
			//data += 4;
			//stun_length += 6;
			std::memcpy(data + stun_length, transcation_id_.c_str(), transcation_id_.size());
			//data += 12;
			stun_length += transcation_id_.size();
			//packet.SetSize(12);
			int32_t padding_bytes = (4- user_name_.size() % 4) % 4;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)kStunAttrUsername);
			stun_length += 2;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)user_name_.size());
			stun_length += 2;
			std::memcpy(data+ stun_length, user_name_.c_str(), user_name_.size());
			stun_length += user_name_.size();
			if (padding_bytes != 0)
			{
				std::memset(data + stun_length , 0, padding_bytes);
				stun_length +=   padding_bytes;
			}
			
			//stun_length += (4 + user_name_.size() + (uint16_t)padding_bytes);
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)kStunAttrXorMappedAddress);
			stun_length += 2;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)8);       //属性长度
			stun_length += 2;
			ByteWriter<uint8_t>::WriteBigEndian(data + stun_length, (uint8_t)0);
			stun_length += 1;
			ByteWriter<uint8_t>::WriteBigEndian(data + stun_length, (uint8_t)0x01);
			stun_length += 1;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)mapped_port_ ^ (kStunMagicCookie >> 16));
			stun_length += 2;
			ByteWriter<uint32_t>::WriteBigEndian(data + stun_length, (uint32_t)(mapped_addr_ ^ kStunMagicCookie));
			stun_length += 4;
			//data += (4 + 8);
			//stun_length += (4 + 8);
			uint8_t    *data_begin = packet.begin();
			size_t  data_bytes = stun_length - 20;
			size_t  paylod_len = data_bytes + ( 4+20);
			ByteWriter<uint16_t>::WriteBigEndian(data_begin + 2, (uint16_t)paylod_len);
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)kStunAttrMessageIntegrity);
			stun_length += 2;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)20);
			stun_length += 2;
			CalcHmac((char*)data + stun_length, (char*)data_begin, data_bytes + 20);
			// 计算完成后，恢复实际长度
			paylod_len = data_bytes + (20+4) + (4 + 4);
			ByteWriter<uint16_t>::WriteBigEndian(data_begin + 2, paylod_len);
			//data += (4 + 20);
			stun_length += (20);
			ByteWriter<uint16_t>::WriteBigEndian(data+ stun_length, (uint16_t)kStunAttrFingerprint);
			stun_length += 2;
			ByteWriter<uint16_t>::WriteBigEndian(data + stun_length, (uint16_t)4);
			stun_length += 2;
			uint32_t crc32 = rtc::ComputeCrc32(data, stun_length-4) ^ 0x5354554e;
			 
			ByteWriter<uint32_t>::WriteBigEndian(data + stun_length, crc32);
			stun_length += 4;
		//	data += (4 + 4);
		//	stun_length += (4 + 4);
			//packet->SetPacketSize(paylod_len + 20);
			packet.SetSize(stun_length);
			return std::move(packet);
		}
		std::string Stun::LocalUFrag()
		{
			auto pos = user_name_.find_first_of(':');
			if (pos != std::string::npos)
			{
				return user_name_.substr(0, pos);
			}
			return std::string();
		}
		void Stun::SetPassword(const std::string &pwd)
		{
			password_ = pwd;
		}
		void Stun::SetMappedAddr(uint32_t addr)
		{
			mapped_addr_ = addr;
		}
		void Stun::SetMappedPort(uint16_t port)
		{
			mapped_port_ = port;
		}
		void Stun::SetMessageType(StunMessageType type)
		{
			type_ = type;
		}
		size_t Stun::CalcHmac(char *buf, const char *data, size_t bytes)
		{
			unsigned int digestLen;
#if OPENSSL_VERSION_NUMBER > 0x10100000L
			HMAC_CTX *ctx = HMAC_CTX_new();
			HMAC_Init_ex(ctx, password_.c_str(), password_.size(), EVP_sha1(), NULL);
			HMAC_Update(ctx, (const unsigned char *)data, bytes);
			HMAC_Final(ctx, (unsigned char *)buf, &digestLen);
			HMAC_CTX_free(ctx);
#else
			HMAC_CTX ctx;
			HMAC_CTX_init(&ctx);
			HMAC_Init_ex(&ctx, password_.c_str(), password_.size(), EVP_sha1(), NULL);
			HMAC_Update(&ctx, (const unsigned char *)data, bytes);
			HMAC_Final(&ctx, (unsigned char *)buf, &digestLen);
			HMAC_CTX_cleanup(&ctx);
#endif
			return digestLen;
		}
	}
}

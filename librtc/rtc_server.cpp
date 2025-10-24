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
#include "libmedia_transfer_protocol/librtc/rtc_server.h"
#include "rtc_base/logging.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/buffer.h"
#include "rtc_base/byte_buffer.h"

namespace libmedia_transfer_protocol {
	namespace librtc {
		namespace {
			static const uint8_t kStunmagicCookie[] = { 0x21, 0x12, 0xA4, 0x42 };
		}
		 RtcServer::RtcServer( ) 
			 :udp_server_(new  libnetwork::UdpServer())
		{

			 // = std::make_unique<libnetwork::UdpServer>();
			 udp_server_->SignalReadPacket.connect(this, &RtcServer::OnRecvPacket);
		}
		 RtcServer::~RtcServer()
		 {
			 if (udp_server_)
			 {
				 udp_server_->SignalReadPacket.disconnect_all();

				 udp_server_.reset();
			 }

		 }

		 bool RtcServer::Start(const char * ip, uint16_t port)
		 {


			 return udp_server_->Startup(ip, port);
			  
		 }
		 int RtcServer::SendPacket(const rtc::Buffer& packet, const rtc::PacketOptions& options)
		 {
			 udp_server_->network_thread()->PostTask(RTC_FROM_HERE, [this]() {
				// udp_server_->SendPacketTo
			 });
			 //if (!udp_server_->network_thread()->IsCurrent())
			 //{
			//	 rtc::Buffer p(packet.data(), packet.size());
			//	 p.SetSize(packet.size());
			//	 network_->PostTask(RTC_FROM_HERE, [this, f = std::move(p), o = std::move(options)]() {
			//		 udp_control_socket_->Send(f.data(), f.size(), o);
			//	 });
			//	 return 0;
			 //}

			 //return udp_control_socket_->Send(packet.data(), packet.size(), options);
			 return 0;

		 }
		 int RtcServer::SendPacketTo(const rtc::Buffer& packet,
			 const rtc::SocketAddress& addr,
			 const rtc::PacketOptions& options)
		 {
			 rtc::Buffer p(packet.data(), packet.size());
			 p.SetSize(packet.size());
			 udp_server_->network_thread()->PostTask(RTC_FROM_HERE, [this,   f = std::move(p), a = std::move(addr), o = std::move(options)]() {
				 udp_server_->SendPacketTo(f, a, o);
			 });
			 return 0;
			 
		 }
		 int RtcServer::SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress & addr, const rtc::PacketOptions & options)
		 {
			  

			 udp_server_->network_thread()->PostTask(RTC_FROM_HERE, [this, f = std::move(packet), a = std::move(addr), o = std::move(options)]() {
				 udp_server_->SendRtpPacketTo(f , a, o);
				 });
				 return 0;
			  
		 }
		 int32_t RtcServer::SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
			 const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		 {
			 
			 udp_server_->network_thread()->PostTask(RTC_FROM_HERE, [this,
					 send_packets = std::move(packets), a = std::move(addr), o = std::move(options)]() {
					 // LIBRTC_LOG(LS_INFO) << "send remote:" << a.ToString();
					 for (const std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>& s : send_packets)
					 {
						 udp_server_->SendTo( s->data(), s->size(), a, o);
					 }
				 });
				 return 0;
			 
			 // return udp_control_socket_->Send(packet.data(), packet.size(), options);
			// return  udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
		 }
		 int RtcServer::SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress & addr, const rtc::PacketOptions & options)
		 {
			 
			 udp_server_->network_thread()->PostTask(RTC_FROM_HERE, [this, f = std::move(packet), a = std::move(addr), o = std::move(options)]() {
				 udp_server_->SendTo(f.data(), f.size(), a, o);
				 });
				 return 0;
			  
		 }
		  
		 void RtcServer::OnRecvPacket(rtc::AsyncPacketSocket * socket, const uint8_t * data, size_t len,
			 const rtc::SocketAddress & addr, const int64_t & ms)
		 {
			// LIBRTC_LOG_T_F(LS_INFO) << "";
			 if (IsStun(data, len))
			 {
				 SignalStunPacket(socket, data, len, addr, ms);
			 }
			 else if (IsDtls(data, len))
			 {
				 SignalDtlsPacket(socket, data, len, addr, ms);
			 }
			 else if (IsRtp(data, len))
			 {
				 SignalRtpPacket(socket, data, len, addr, ms);
			 }
			 else if (IsRtcp(data, len))
			 {
				 SignalRtcpPacket(socket, data, len, addr, ms);
			 }
			 else
			 {
				 LIBRTC_LOG_T_F(LS_WARNING) << " recv unk type packet addr:" << addr.ToString();
			 } 
		 }
		 
		 bool RtcServer::IsStun(const uint8_t * data, int32_t len)
		 {
			 // clang-format off
			 return (
				 // STUN headers are 20 bytes.
				 (len >= 20) &&
				 // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
				 (data[0] < 3) &&
				 // Magic cookie must match.
				 (data[4] == kStunmagicCookie[0]) && (data[5] == kStunmagicCookie[1]) &&
				 (data[6] == kStunmagicCookie[2]) && (data[7] == kStunmagicCookie[3])
				 );
			 // clang-format on
			// return len >= 20 && data[0] >= 0 && data[0] <= 3;
		 }
		 bool RtcServer::IsDtls(const uint8_t * data, int32_t len)
		 {
			 // clang-format off
			 return (
				 // Minimum DTLS record length is 13 bytes.
				 (len >= 13) &&
				 // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
				 (data[0] > 19 && data[0] < 64)
				 );
			 // clang-format on
			 //return len >= 13 && data[0] >= 20 && data[0] <= 63;
		 }
		 bool RtcServer::IsRtp(const uint8_t * data, int32_t len)
		 {
			 uint8_t pt = (uint8_t)data[1];
			 // clang-format off
			 return (
				 (len >= 12) &&
				 // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
				 (data[0] > 127 && data[0] < 192) &&
				 // RTP Version must be 2.
				 (data[0] & 0x80)
				 );
			 // clang-format on
			 //return len >= 12 && data[0] & 0x80 && !(pt >= 192 && pt <= 223);
		 }
		 bool RtcServer::IsRtcp(const uint8_t * data, int32_t len)
		 {
			 /* Struct for RTCP common header. */
			 struct CommonHeader
			 {
#if defined(MS_LITTLE_ENDIAN)
				 uint8_t count : 5;  // 一个包中Report Block个数
				 uint8_t padding : 1;// 填充标识， 最后一个填充字节是（）个数
				 uint8_t version : 2;
#elif defined(MS_BIG_ENDIAN)
				 uint8_t version : 2;
				 uint8_t padding : 1;
				 uint8_t count : 5;
#endif
				 uint8_t packetType : 8; // 不同RTCP包的类型
				 uint16_t length : 16;   // 16位，包长度（包括头）。[数值为（N-1）个4字节]
			 };
			 // clang-format off
			 uint8_t pt = (uint8_t)data[1];
			 return (
				 (len >= 12) &&
				 // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
				 (data[0] > 127 && data[0] < 192) &&
				 // RTP Version must be 2.
				 //(header->version == 2) &&
				 (data[0] & 0x80) &&
				 // RTCP packet types defined by IANA:
				 // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml#rtp-parameters-4
				 // RFC 5761 (RTCP-mux) states this range for secure RTCP/RTP detection.
				 (pt >= 192 && pt <= 223)
				 );
			 // clang-format on
			 //uint8_t pt = (uint8_t)data[1];
			// return len >= 12 && data[0] & 0x80 && (pt >= 192 && pt <= 223);
		 }
		 
	}
}

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
				   date:  2025-10-05



 ******************************************************************************/
#include "libmedia_transfer_protocol/librtsp/rtsp_session.h"
#ifdef WIN32
#include "rtc_base/win32_socket_server.h"
#endif
#include "rtc_base/string_encode.h"
#include "rtc_base/thread.h"
#include "rtc_base/logging.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/internal/default_socket_server.h"
#include "libice/stun.h"
#include "rtc_base/third_party/base64/base64.h"
#include "rtc_base/message_digest.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_util.h"
#include "api/array_view.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/common_header.h"
#include "rtc_base/string_encode.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
namespace libmedia_transfer_protocol
{
	namespace librtsp
	{

		namespace {


			static const char * kUserAgent = "libmedia_transfer_protocol/1.0 (CRTC 1.0)";
			static const char * kRtspVersion = " RTSP/1.0";
			static const char * kRtspEnd = "\r\n";
			static const char * kCSeq = "CSeq: ";
			rtc::Socket* CreateClientSocket(int family) {
				//rtc::Thread* thread = rtc::Thread::Current();
				//rtc::PhysicalSocket *sock = new rtc::PhysicalSocket((rtc::PhysicalSocketServer*)thread->socketserver() );
				//rtc::PhysicalSocketServer  sock =  rtc::PhysicalSocketServer();
				//sock->Create(family, SOCK_STREAM);
				//return sock;
				//new Thread(CreateDefaultSocketServer())

				//rtc::Thread* thread = rtc::Thread::Current();
				//RTC_DCHECK(thread != NULL);
				//return thread->socketserver()->CreateSocket(family, SOCK_STREAM);
#ifdef WIN32
				rtc::Win32Socket* sock = new rtc::Win32Socket();
				sock->CreateT(family, SOCK_STREAM);
			//	sock->CreateSink();
				return sock;
#elif defined(WEBRTC_POSIX)
				rtc::Thread* thread = rtc::Thread::Current();
				RTC_DCHECK(thread != NULL);
				return thread->socketserver()->CreateSocket(family, SOCK_STREAM);
#else
#error Platform not supported.
#endif
			}
		}
		RtspSession::RtspSession()
			:network_thread_(new rtc::Thread(rtc::CreateDefaultSocketServer()))
			, recv_buffer_(1024*1024)
			//, physical_socket_server_(std::make_unique<rtc::PhysicalSocketServer>())
		{
			network_thread_->SetName("rtsp_session_socket", nullptr);
			network_thread_->Start();


			std::string ha1_digest;
			std::string ha1 = "admin:IP Camera(FB997):Cs@563519" ;
			bool size = rtc::ComputeDigest(rtc::DIGEST_MD5, ha1,
				&ha1_digest);

			// 10dbbfd5d656ee6e113a0a1eb95dca94
			//std::string hash_d = rtc::hex_encode(ha1_digest);

			std::string ha2_digest;
			std::string ha2 = "DESCRIBE" + std::string(":") + "rtsp://192.168.1.64:554/streaming/channels/101";
			size = rtc::ComputeDigest(rtc::DIGEST_MD5, ha2,
				&ha2_digest);


			std::string  digest;
			std::string input = ha1_digest + std::string(":") + "83be737756675f8f99c8e74e8e7df845" + std::string(":") + ha2_digest;
			size = rtc::ComputeDigest(rtc::DIGEST_MD5, input,
				&digest);
			RTC_LOG(LS_INFO) << "digest:" << digest;

			callback_map_["Content-Type"] = &RtspSession::HandlerContentType;
			callback_map_["Session"] = &RtspSession::HandlerSession;

		}
		RtspSession::~RtspSession()
		{
			network_thread_->Invoke<void>(RTC_FROM_HERE, [this](){
				Close();
			});
			network_thread_->Stop();
			network_thread_.reset();
		}
		bool RtspSession::Play(const std::string & url)
		{
			//rtsp://admin:Cs@563519@192.168.1.64/streaming/channels/101
			std::vector<std::string> fields;
			rtc::split(url, '/', &fields);
			if (fields.size() < 2) 
			{
				return false;
			}
			std::remove_if(fields.begin(), fields.end(), []  (const std::string & s) {
				if (s.empty())
				{
					return true;
				}
				return false;
			});
			protocol_name_ = fields[0];
			for (const std::string & data : fields)
			{
				RTC_LOG(LS_INFO) << " " << data;
			}

			//分格i用户名和密码
			std::vector<std::string>   url_fields;
			rtc::split(fields[1], '@', &url_fields);
			std::string ip_data;
			if (url_fields.size() > 1)
			{
				//说明有用户名和密码
				//拿出用户名和密码
				// admin:Cs@563519
				std::string temp_user_info   ;
				temp_user_info = url_fields[0];
				for (size_t i = 1; i < url_fields.size()-1; ++i)
				{
					//补充@符号
					temp_user_info += "@";
					temp_user_info += url_fields[i];
				}
				std::vector<std::string>   user_info;
				//  分格符 :
				rtc::split(temp_user_info, ':', &user_info);
				if (user_info.size() < 2)
				{
					RTC_LOG(LS_ERROR) << "user info paser filaed !!!" << url;
					return false;
				}
				user_name_ = user_info[0];
				password_ = user_info[1]; 
			}
			ip_data = url_fields[url_fields.size() - 1];
			std::vector<std::string>  ip_fields;
			rtc::split(ip_data, '@', &ip_fields);
			server_address_.SetIP(ip_fields[0]);
			if (ip_fields.size() < 2)
			{
				// 默认port  544
				server_address_.SetPort(554);
			}
			else
			{
				server_address_.SetPort(std::atoi(ip_fields[1].c_str()));
			}


			//保存参数发送
			for (size_t i = 2; i < fields.size(); ++i)
			{
				if (fields[i].empty())
				{
					continue;
				}
				params_.push_back(fields[i]);
				url_params_ += "/" + fields[i];
			}
			stream_url_ = "rtsp://" + server_address_.hostname() + ":" + std::to_string( server_address_.port() )+ url_params_ ;
			Connect();
			return true;
		}
		bool RtspSession::Push(const std::string & url)
		{
			return false;
		}
		void RtspSession::SendOptions(rtc::Socket* socket)
		{
			/*
			OPTIONS rtsp://192.168.1.64:554/streaming/channels/101 RTSP/1.0
			CSeq: 2
			User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
			*/
			std::stringstream cmd;
			cmd << "OPTIONS "<< stream_url_ << kRtspVersion << kRtspEnd;
			cmd << kCSeq << ++cseq_ << kRtspEnd;
			cmd << "User-Agent: " << kUserAgent << kRtspEnd;
			cmd << kRtspEnd;
			// send 
			RTC_LOG(LS_INFO) << cmd.str();
			size_t sent = socket->Send(cmd.str().c_str(), cmd.str().length());
			RTC_DCHECK(sent == cmd.str().length());
		}
		void RtspSession::SendDescribe(rtc::Socket * socket)
		{

		/*
		
			DESCRIBE rtsp://192.168.1.64:554/streaming/channels/101 RTSP/1.0
			CSeq: 3
			User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
			Accept: application/sdp


			DESCRIBE rtsp://192.168.1.64:554/streaming/channels/101 RTSP/1.0
			CSeq: 4
			Authorization: Digest username="admin", realm="IP Camera(FB997)", nonce="2d9e65274efe1e65d68aa5126b04a7fa", uri="rtsp://192.168.1.64:554/streaming/channels/101", response="b7907da6f3b9387787c3a39e68a3a6e7"
			User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)
			Accept: application/sdp

		*/
			std::stringstream cmd;
			cmd << "DESCRIBE rtsp://" << server_address_.hostname() << ":" << server_address_.port() << url_params_ << kRtspVersion << kRtspEnd;
			cmd << kCSeq << ++cseq_ << kRtspEnd;
#if 0
			if (!realm_.empty())
			{
				cmd << "Authorization: Digest username=\"" << user_name_<< "\"";
				cmd << ", realm=\"" << realm_ << "\"";
				cmd << ", nonce=\"" << nonce_ << "\"";
				cmd << ", uri=\"" << stream_url_ << "\"";
				cmd << ", response=\"" << build_response("DESCRIBE") <<"\"";
				cmd << kRtspEnd;
			}
#endif // 
			if (!realm_.empty())
			{
				cmd << BuildAuthorization("DESCRIBE") << kRtspEnd;

			}
			cmd << "User-Agent: " << kUserAgent << kRtspEnd;
			cmd << "Accept: application/sdp" << kRtspEnd;
			cmd << kRtspEnd;
			RTC_LOG(LS_INFO) << cmd.str();
			// send 
			size_t sent = socket->Send(cmd.str().c_str(), cmd.str().length());
			RTC_DCHECK(sent == cmd.str().length());
		}
		void RtspSession::SendSetup(rtc::Socket * socket)
		{
			/*
			
				SETUP rtsp://192.168.1.64:554/streaming/channels/101/trackID=1 RTSP/1.0
				Transport: RTP/AVP/TCP;unicast;interleaved=0-1
				CSeq: 4
				User-Agent: Lavf60.20.100
				Authorization: Digest username="admin", realm="IP Camera(FB997)", nonce="b2f25195ee36d5175aa7eab458689cf9", uri="rtsp://192.168.1.64:554/streaming/channels/101/trackID=1", response="08853f94b6bd19579bf7f81784470de2"

			*/
			if (track_control_.empty())
			{
				return;
			}
			int32_t trank_interleaved = track_control_.size();
			std::string track = track_control_.front();
			track_control_.pop_front();
			//std::remove(track_control_.begin(), track_control_.end(), track);
			

			std::stringstream cmd;

			cmd << "SETUP " << track << kRtspVersion << kRtspEnd;
			cmd << "Transport: RTP/AVP/TCP;unicast;interleaved=";
			if (trank_interleaved > 1)
			{
				cmd << "0-1" << kRtspEnd;
			}
			else
			{
				cmd << "2-3" << kRtspEnd;
			}
			cmd << kCSeq << ++cseq_ << kRtspEnd;
			cmd << "User-Agent: " << kUserAgent << kRtspEnd;

			if (!session_id_.empty())
			{
				cmd << "Session: " << session_id_ << kRtspEnd;
			}

			if (!realm_.empty())
			{
				cmd << BuildAuthorization("SETUP") << kRtspEnd;

			}
			cmd << kRtspEnd;
			RTC_LOG(LS_INFO) << cmd.str();
			// send 
			size_t sent = socket->Send(cmd.str().c_str(), cmd.str().length());
			RTC_DCHECK(sent == cmd.str().length());

		}
		void RtspSession::SendPlay(rtc::Socket * socket)
		{
			/*
			PLAY rtsp://192.168.1.64:554/streaming/channels/101/ RTSP/1.0
				Range: npt=0.000-
				CSeq: 6
				User-Agent: Lavf60.20.100
				Session: 1225743327
			*/
			std::stringstream cmd;

			cmd << "PLAY " << stream_url_ << kRtspEnd;
			cmd << "Range: npt=0.000-" << kRtspEnd; 
			cmd << kCSeq << ++cseq_ << kRtspEnd;
			cmd << "User-Agent: " << kUserAgent << kRtspEnd;

			if (!session_id_.empty())
			{
				cmd << "Session: " << session_id_ << kRtspEnd;
			}

			if (!realm_.empty())
			{
				cmd << BuildAuthorization("PLAY") << kRtspEnd;

			}
			cmd << kRtspEnd;
			RTC_LOG(LS_INFO) << cmd.str();
			// send 
			size_t sent = socket->Send(cmd.str().c_str(), cmd.str().length());
			RTC_DCHECK(sent == cmd.str().length());
		}
		void RtspSession::HandlerContentType(rtc::Socket * socket, std::vector<std::string> data)
		{
			//解析sdp 信息
			if (data.size() < 5)
			{
				RTC_LOG(LS_ERROR) << " parse sdp filed !!!";
				return;
			}
			libp2p_peerconnection::ContentInfo * content_info = nullptr;
			for (size_t i = 4; i < data.size(); ++i)
			{
				std::vector<std::string>  splited;
				rtc::split(data[i], '=', &splited);
				if (splited.size() < 2)
				{
					continue;
				}

				//find m= xxx 
				if (splited[0] == "m")
				{
					//m=video 0 RTP/AVP 96
					//判断之前有没有new 有就保存数据
					if (content_info)
					{
						session_description_.contents_.emplace_back(*content_info);
						delete content_info;
						content_info = nullptr;
					}
					content_info = new libp2p_peerconnection::ContentInfo();
					std::vector<std::string>  m_split;
					rtc::split(splited[1], ' ', & m_split);
					if (m_split.size() > 3)
					{
						content_info->name = m_split[0];
						if (m_split[0] == "video")
						{
							//content_info->description_ = std::make_unique< libp2p_peerconnection::VideoContentDescription>();
							//content_info->description_->rtcp_reduced_size_
							auto video_desc = std::make_unique< libp2p_peerconnection::VideoContentDescription>();
							VideoCodec video_codec;
							video_codec.id = std::atoi(m_split[3].c_str());
							video_codec.name = m_split[0];
							video_desc->codecs_.push_back(video_codec);
							content_info->description_ = std::move(video_desc);
						}
						else
						{
							//content_info->description_ = std::make_unique< libp2p_peerconnection::VideoContentDescription>();
							//content_info->description_->rtcp_reduced_size_
							auto audio_desc = std::make_unique< libp2p_peerconnection::AudioContentDescription>();
							AudioCodec audio_codec;
							audio_codec.id = std::atoi(m_split[3].c_str());
							audio_codec.name = m_split[0];
							audio_desc->codecs_.push_back(audio_codec);
							content_info->description_ = std::move(audio_desc);
						}
						
					}
				}
				else if (splited[0] == "a" && content_info)
				{
					std::vector<std::string>  a_split;
					rtc::split(splited[1], ':', &a_split);
					if (a_split.size() > 1)
					{
						if (a_split[0] == "control")
						{
							std::string trank_name = a_split[1];
							for (size_t w = 2; w < a_split.size(); ++w)
							{
								trank_name += ":" + a_split[w];
							}
							trank_name += "=" + splited[2];
							track_control_.push_back(trank_name);
						}
					}
					// a=x-dimensions:1280,720
					//a=control:rtsp://192.168.1.64:554/streaming/channels/101/trackID=1
					//a=rtpmap:96 H264/90000
					//a = fmtp:96 profile - level - id = 420029; packetization - mode = 1; sprop - parameter - sets = Z2QAH60AGxqAUAW6bgICAoAAA4QAAK / IAg == , aO44sA ==
				}

			}

			SendSetup(socket);
		}
		void RtspSession::HandlerSession(rtc::Socket * socket, std::vector<std::string> data)
		{


			if (data.size() < 3)
			{
				RTC_LOG(LS_ERROR) << "data size to samlll !!! size :" << data.size();
				return;
			}
			std::vector<std::string>  s_split;
			rtc::split(data[2], ':', &s_split);
			std::vector<std::string> session_split;
			rtc::split(s_split[1], ';', &session_split);
			session_id_ = session_split[0];
			if (track_control_.empty())
			{
				SendPlay(socket);
			}
			else 
			{
				SendSetup(socket);
			}

		}
		void RtspSession::Connect()
		{
			if (state_ != NOT_CONNECTED) {
				RTC_LOG(WARNING)
					<< "The client must not be connected before you can call Connect()";
				//callback_->OnServerConnectionFailure();
				return;
			}
			if (server_address_.IsUnresolvedIP()) {
				state_ = RESOLVING;
				resolver_ = new rtc::AsyncResolver();
				resolver_->SignalDone.connect(this, &RtspSession::OnResolveResult);
				resolver_->Start(server_address_);
			}
			else {
				DoConnect();
			}

		}
		void RtspSession::OnConnect(rtc::Socket * socket)
		{
			RTC_LOG_F(LS_INFO) << "";
			//SendOptions(socket);
			state_ = CONNECTED;
		}
		void RtspSession::OnClose(rtc::Socket * socket, int ret)
		{
			RTC_LOG_F(LS_INFO) << "";
		}
		void RtspSession::OnRead(rtc::Socket * socket)
		{
			
			//char buffer[0xFFFF] = {0};
			//std::string data;
			rtc::Buffer buffer(1024 * 1024);
			
			//recv_buffer_size_ = read_bytes - paser_size;
			rtc::ArrayView<uint8_t> array_buffer(buffer.begin(), buffer.capacity());
			int32_t  read_bytes = 0;
			if (recv_buffer_size_ > 0)
			{
				memcpy((char *)buffer.begin(), recv_buffer_.begin(), recv_buffer_size_);
				read_bytes = recv_buffer_size_;
				recv_buffer_size_ = 0;
			}
			do {
				int bytes = socket->Recv(buffer.begin()+ read_bytes, buffer.capacity(), nullptr);
				if (bytes <= 0)
					break;
				read_bytes += bytes;
			} while (true);
			RTC_LOG_F(LS_INFO) << "recv data : " << read_bytes;
			
			
			//判断rtsp/rtcp/rtp的协议
			int32_t paser_size = 0;
			while (read_bytes - paser_size > 0)
			{
				uint8_t type_protocol_ = libmedia_transfer_protocol::ByteReader<uint8_t>::ReadBigEndian((&buffer.begin()[paser_size]));
				if ('$' == type_protocol_)
				{
					RTC_LOG(LS_INFO) << "$$$$$$";
					// rtsp 包装rtp包
					 //   RtspMagic *rtsp_magic = static_cast<RtspMagic *>(reinterpret_cast<  RtspMagic *>(buffer.begin() + paser_size)); //(buffer.begin()+paser_size);
					//rtsp_magic->length_   = ((buffer.begin()[paser_size]) >> 16) << 16;
					//rtsp_magic->length_ += (uint8_t)
					RtspMagic rtsp_magic;
					
#if __linux__
					rtsp_magic.magic_ = libmedia_transfer_protocol::ByteReader<uint8_t>::ReadBigEndian((&buffer.begin()[paser_size]));
					rtsp_magic.channel_ = libmedia_transfer_protocol::ByteReader<uint8_t>::ReadBigEndian((&buffer.begin()[paser_size + 1]));
					rtsp_magic.length_ = libmedia_transfer_protocol::ByteReader<uint16_t>::ReadBigEndian((&buffer.begin()[paser_size+2]));
#else 
					rtsp_magic.magic_ = libmedia_transfer_protocol::ByteReader<uint8_t>::ReadBigEndian((&buffer.begin()[paser_size]));
					rtsp_magic.channel_ = libmedia_transfer_protocol::ByteReader<uint8_t>::ReadBigEndian((&buffer.begin()[paser_size + 1]));
					rtsp_magic.length_ = libmedia_transfer_protocol::ByteReader<uint16_t>::ReadBigEndian((&buffer.begin()[paser_size + 2]));
#endif 
					//memcpy(&rtsp_magic, (buffer.begin() + paser_size), sizeof(RtspMagic));
					//包完整性判断
					// memcopy(recv_buffer_.begin(), buffer.begin() + ;
					if (rtsp_magic.length_ > (read_bytes - paser_size - sizeof(RtspMagic)))
					{
						RTC_LOG(LS_INFO) << "recv datasize:" << read_bytes - paser_size - sizeof(RtspMagic) << ", rtsp magic length :" << rtsp_magic.length_;
						//包读取不完整待定下次数据
						memcpy((char *)recv_buffer_.begin(), buffer.begin() + paser_size, (read_bytes - paser_size));
						recv_buffer_size_ = read_bytes - paser_size;
						return;
					}
					paser_size += sizeof(RtspMagic);
					//  RTP 中  padding 
					if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + paser_size, rtsp_magic.length_)))
					{
						libmedia_transfer_protocol::RtpPacket  rtp_packet;
						bool ret = rtp_packet.Parse(buffer.begin() + paser_size, rtsp_magic.length_/*read_bytes - paser_size*/);
						if (!ret)
						{

							RTC_LOG(LS_WARNING) << "rtp parse failed !!! size:" << (read_bytes - paser_size); //<< "  , hex :" << rtc::hex_encode((const char *)(buffer.begin() + paser_size), (size_t)(read_bytes - paser_size));
						}
						else
						{
							// RTSP IntrerleaveFrame   length 
							paser_size += rtsp_magic.length_;
							RTC_LOG(LS_INFO) << "rtp info :" << rtp_packet.ToString();
						}
					}
					else if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + paser_size, rtsp_magic.length_/*read_bytes - paser_size*/)))
					{
						libmedia_transfer_protocol::rtcp::CommonHeader rtcp_block;  //rtcp_packet;
						bool ret = rtcp_block.Parse(buffer.begin() + paser_size, rtsp_magic.length_/* read_bytes - paser_size*/);
						if (!ret)
						{
							RTC_LOG(LS_WARNING) << "rtcp parse failed !!!";
						}
						else
						{
							paser_size += rtsp_magic.length_;
							RTC_LOG(LS_INFO) << "rtcp info :" << rtcp_block.ToString();
						}
					}
				}
				else if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + paser_size, read_bytes - paser_size)))
				{
					libmedia_transfer_protocol::RtpPacket  rtp_packet;
					bool ret = rtp_packet.Parse(buffer.begin() + paser_size, read_bytes - paser_size);
					if (!ret)
					{
						RTC_LOG(LS_WARNING) << "rtp parse failed !!!";
					}
					else
					{
						paser_size += rtp_packet.size();
						RTC_LOG(LS_INFO) << "rtp info :" << rtp_packet.ToString();
					}
				}
				else if (libmedia_transfer_protocol::IsRtpPacket(rtc::ArrayView<uint8_t>(buffer.begin() + paser_size, read_bytes - paser_size)))
				{
					libmedia_transfer_protocol::rtcp::CommonHeader rtcp_block;  //rtcp_packet;
					bool ret = rtcp_block.Parse(buffer.begin() + paser_size, read_bytes - paser_size);
					if (!ret)
					{
						RTC_LOG(LS_WARNING) << "rtp parse failed !!!";
					}
					else
					{
						paser_size += rtcp_block.packet_size();
						RTC_LOG(LS_INFO) << "rtcp info :" << rtcp_block.ToString();
					}
				}
				else
				{

					// 解析rtsp返回的格式数据
					std::string data((char *)buffer.begin() + paser_size, read_bytes - paser_size);
					std::vector<std::string>  fileds;
					rtc::tokenize(data, '\n', &fileds);
					// 第一行   RTSP/1.0 401 Unauthorized
					std::vector<std::string> one_line;
					rtc::split(fileds[0], ' ', &one_line);
					if (one_line.size() < 2)
					{
						RTC_LOG(LS_ERROR) << " parse failed !!! " ;
						return;
					}
					if (fileds.size() < 3)
					{
						RTC_LOG(LS_ERROR) << " rtsp relay  parse failed !!! ";
						return;
					}
					//获取状态码
					int32_t code = std::atoi(one_line[1].c_str());
					//401 说明需要发送验证数据
					// CSeq: number
					std::vector<std::string> two_line;
					rtc::split(fileds[1], ':', &two_line);
					// WWW-Authenticate: Digest realm="IP Camera(FB997)", nonce="2d9e65274efe1e65d68aa5126b04a7fa", stale="FALSE"
									// 获取用户名
					std::vector<std::string> three_line;
					
					rtc::split(fileds[2], ':', &three_line);
					if (code == 401)
					{

						std::vector<std::string> three_line_params;
						rtc::split(three_line[1], '"', &three_line_params);
						realm_ = three_line_params[1];
						nonce_ = three_line_params[3];
						/*
						const std::string& username,
									   const std::string& realm,
									   const std::string& password,
									   std::string* hash
						*/
						/*
						服务器返回 401 Unauthorized，并附带 Digest realm、nonce 等参数；

						客户端根据算法计算 response = MD5( MD5(username:realm:password) : nonce : MD5(method:uri) )；

						HA1 = MD5(username:realm:password)
						HA2 = MD5(method:uri)
						response = MD5(HA1:nonce:HA2)
						*/
						//libice::ComputeStunCredentialHash(user_name_, realm_, nonce_, &response_);

						//char ha1_digest[32] = {0};

						//md5();
						// HA1 = MD5(username:realm:password)

						// 再次发送 DESCRIBE指令
						SendDescribe(socket);
					}
					if (three_line[0] == "Public")
					{
						//发送Descript指令
						SendDescribe(socket);
					}

					auto iter = callback_map_.find(three_line[0]);
					if (iter == callback_map_.end())
					{
						RTC_LOG(LS_INFO) << "not find method:" << three_line[0];
					}
					else
					{
						(this->*(iter->second))(socket, std::move(fileds));
					}
					break;
				}
			}
			//memcopy(recv_buffer_.begin(), buffer.begin() + ;
			//std::string data((char*)buffer.begin(), read_bytes);



		}
		void RtspSession::OnWrite(rtc::Socket * socket)
		{
			RTC_LOG_F(LS_INFO) << "";
			SendOptions(socket);
		}
		void RtspSession::OnResolveResult(rtc::AsyncResolverInterface * resolver)
		{
			if (resolver_->GetError() != 0) {
				//callback_->OnServerConnectionFailure();
				resolver_->Destroy(false);
				
				resolver_ = NULL;
				state_ = NOT_CONNECTED;
			}
			else {
				server_address_ = resolver_->address();
				DoConnect();
			}
		}
		void RtspSession::DoConnect()
		{
			//control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
			control_socket_.reset(network_thread_->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_STREAM));
			//control_socket_->Accept(control_sock)
			InitSocketSignals();
			bool ret = ConnectControlSocket();
			if (ret)
			{
				state_ = CONNECTING;
				//((rtc::Win32Socket*)())
				//SendOptions(control_socket_.get());
				//physical_socket_server_->Add( dynamic_cast<rtc::Dispatcher*>( control_socket_.get()));
			}
			if (!ret) {
				//callback_->OnServerConnectionFailure();
				RTC_LOG(LS_ERROR) << "connect failed !!!";
			}
		}
		void RtspSession::InitSocketSignals()
		{
			control_socket_->SignalCloseEvent.connect(this, &RtspSession::OnClose); 
			control_socket_->SignalConnectEvent.connect(this, &RtspSession::OnConnect); 
			control_socket_->SignalReadEvent.connect(this, &RtspSession::OnRead);
			control_socket_->SignalWriteEvent.connect(this, &RtspSession::OnWrite);
		}
		bool RtspSession::ConnectControlSocket()
		{
			RTC_LOG(LS_INFO) << "connect ->>" << server_address_.ToString();
			int err = control_socket_->Connect(server_address_);
			if (err == SOCKET_ERROR) {
				Close();
				return false;
			}
			//control_socket_->SetTimeout(5);
			//control_socket_->Attach(control_socket_->)
			return true;
		}
		void RtspSession::Close()
		{
			control_socket_->Close();
			  
			if (resolver_ != NULL) {
				resolver_->Destroy(false);
				delete resolver_;
				resolver_ = NULL;
			}
			 
			state_ = NOT_CONNECTED;
		}
		std::string RtspSession::BuildAuthorization(const std::string & method)
		{

			std::stringstream cmd;
			


			std::string ha1_digest;
			std::string ha1 = user_name_ + ":" + realm_ + ":" + password_;
			bool size = rtc::ComputeDigest(rtc::DIGEST_MD5, ha1,
				&ha1_digest);

			// 10dbbfd5d656ee6e113a0a1eb95dca94
			//std::string hash_d = rtc::hex_encode(ha1_digest);

			std::string ha2_digest;
			std::string ha2 = method + std::string(":") + stream_url_;
			size = rtc::ComputeDigest(rtc::DIGEST_MD5, ha2,
				&ha2_digest);

			response_.clear();
			//std::string  digest;
			std::string input = ha1_digest + std::string(":") + nonce_ + std::string(":") + ha2_digest;
			size = rtc::ComputeDigest(rtc::DIGEST_MD5, input,
				&response_);
			//response_ = rtc::Base64::Encode(response_);
			//response_ = digest;// rtc::hex_encode(digest);
			cmd << "Authorization: Digest username=\"" << user_name_ << "\"";
			cmd << ", realm=\"" << realm_ << "\"";
			cmd << ", nonce=\"" << nonce_ << "\"";
			cmd << ", uri=\"" << stream_url_ << "\"";
			cmd << ", response=\"" << response_ << "\"";
		 
			return cmd.str();
		}
	}
}
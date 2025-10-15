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
				   date:  2025-10-16



 ******************************************************************************/
#include "libmedia_transfer_protocol/librtc/dtls.h"
#include "rtc_base/logging.h"
namespace libmedia_transfer_protocol {

	namespace librtc
	{

		//Dtls::Dtls(/*DtlsHandler *handler*/)
		//	//:handler_(handler)
		//{

		//}
		Dtls::~Dtls()
		{
			if (ssl_context_)
			{
				SSL_CTX_free(ssl_context_);
				ssl_context_ = nullptr;
			}
			if (ssl_)
			{
				SSL_free(ssl_);
				ssl_ = nullptr;
				bio_read_ = nullptr;
				bio_write_ = nullptr;
			}
		}
		bool Dtls::Init()
		{
			auto ret = dtls_cert_.Init();
			if (!ret)
			{
				return false;
			}
			ret = InitSSLContext();
			if (!ret)
			{
				return false;
			}
			ret = InitSSL();
			return true;
		}
		void Dtls::OnRecv(const char *data, uint32_t size)
		{
			BIO_reset(bio_read_);
			BIO_reset(bio_write_);

			BIO_write(bio_read_, data, size);
			SSL_do_handshake(ssl_);

			NeedPost();

			if (is_done_)
			{
				GetSrtpKey();
				 
				SignalDtlsHandshakeDone(this);
				return;
			}
			SSL_read(ssl_, buffer_, 65535);
		}
		const std::string &Dtls::Fingerprint() const
		{
			return dtls_cert_.Fingerprint();
		}
		void Dtls::SetDone()
		{
			is_done_ = true;
		}
		void Dtls::SetClient(bool client)
		{
			is_client_ = true;
		}
		const std::string &Dtls::SendKey()
		{
			return send_key_;
		}
		const std::string &Dtls::RecvKey()
		{
			return recv_key_;
		}
		bool Dtls::InitSSLContext()
		{
			ssl_context_ = SSL_CTX_new(DTLS_method());
			//设置证书
			SSL_CTX_use_certificate(ssl_context_, dtls_cert_.GetCerts());
			//设置私钥
			auto ret = SSL_CTX_use_PrivateKey(ssl_context_, dtls_cert_.GetPrivateKey());
			if (!ret)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "SSL_CTX_use_PrivateKey failed.";
				return false;
			}
			ret = SSL_CTX_check_private_key(ssl_context_);
			if (!ret)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "SSL_CTX_check_private_key failed.";
				return false;
			}
			SSL_CTX_set_cipher_list(ssl_context_, "ALL");
			SSL_CTX_set_verify(ssl_context_, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, Dtls::SSLVerify);
			SSL_CTX_set_info_callback(ssl_context_, Dtls::SSLInfo);

			// 设置基本
			SSL_CTX_set_verify_depth(ssl_context_, 4);
			SSL_CTX_set_read_ahead(ssl_context_, 1);

			SSL_CTX_set_tlsext_use_srtp(ssl_context_, "SRTP_AES128_CM_SHA1_80");
			return true;
		}
		bool Dtls::InitSSL()
		{
			ssl_ = SSL_new(ssl_context_);
			if (!ssl_)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "SSL_new failed.";
				return false;
			}

			SSL_set_ex_data(ssl_, 0, static_cast<void*>(this));

			bio_read_ = BIO_new(BIO_s_mem());
			bio_write_ = BIO_new(BIO_s_mem());

			SSL_set_bio(ssl_, bio_read_, bio_write_);

			SSL_set_mtu(ssl_, 1350);
			SSL_set_accept_state(ssl_);
			return true;
		}
		int Dtls::SSLVerify(int preverify_ok, X509_STORE_CTX *ctx)
		{
			return 1;
		}
		void Dtls::SSLInfo(const SSL* ssl, int where, int ret)
		{
			Dtls *dtls = static_cast<Dtls*>(SSL_get_ex_data(ssl, 0));
			int w = where & ~SSL_ST_MASK;

			if (w & SSL_ST_CONNECT)
			{
				dtls->SetClient(true);
			}
			else if (w & SSL_ST_ACCEPT)
			{
				dtls->SetClient(false);
			}
			else
			{
				dtls->SetClient(false);
			}

			if (where & SSL_CB_HANDSHAKE_DONE)
			{
				dtls->SetDone();
				LIBRTC_LOG(LS_INFO) << "dtls handshake done.";
			}
		}
		void Dtls::NeedPost()
		{
			if (BIO_eof(bio_write_))
			{
				return;
			}

			char *data = nullptr;

			auto read = BIO_get_mem_data(bio_write_, &data);
			if (read <= 0)
			{
				return;
			}


			SignalDtlsSendPakcet(data, read, this);
			 
			BIO_reset(bio_write_);
		}
		void Dtls::GetSrtpKey()
		{
			int32_t srtp_key_len = 16;
			int32_t srtp_salt_len = 14;

			unsigned char material[30 * 2];
			static std::string lable = "EXTRACTOR-dtls_srtp";
			auto ret = SSL_export_keying_material(ssl_, material, sizeof(material),
				lable.c_str(), lable.size(), NULL, 0, 0);
			if (ret <= 0)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "SSL_export_keying_material failed.ret:" << ret;
			}

			int32_t offset = 0;
			std::string client_master_key((char*)material, srtp_key_len);
			offset += srtp_key_len;
			std::string server_mater_key((char*)(material + offset), srtp_key_len);
			offset += srtp_key_len;
			std::string client_master_salt((char*)(material + offset), srtp_salt_len);
			offset += srtp_salt_len;
			std::string server_master_salt((char*)(material + offset), srtp_salt_len);
			offset += srtp_salt_len;

			LIBRTC_LOG(LS_INFO) << " dtls is server.";
			recv_key_ = client_master_key + client_master_salt;
			send_key_ = server_mater_key + server_master_salt;
		}


	}

}

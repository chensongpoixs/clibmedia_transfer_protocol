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
#include <openssl/asn1.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <WinSock2.h>
#include "rtc_base/thread.h"
namespace libmedia_transfer_protocol {

	namespace libssl
	{
		namespace
		{
			inline static unsigned int OnSslDtlsTimer(SSL* /*ssl*/, unsigned int timerUs)
			{
				if (timerUs == 0)
				{
					return 100000;
				}
				else if (timerUs >= 4000000)
				{
					return 4000000;
				}
				//else
					return 2 * timerUs;
			}
		}

		//Dtls::Dtls(/*DtlsHandler *handler*/)
		//	//:handler_(handler)
		//{

		Dtls::Dtls(webrtc::TaskQueueFactory * task_queue_factory)
			: dtls_queue_(task_queue_factory->CreateTaskQueue("dtls_queue", webrtc::TaskQueueFactory::Priority::NORMAL))

		{
			//InitSSL();
			ssl_ = SSL_new(DtlsCerts::GetInstance().GetSslCtx());
			if (!ssl_)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "SSL_new failed.";
				goto error;
			}
			//注册数据回调
			SSL_set_ex_data(ssl_, 0, static_cast<void*>(this));

			bio_read_ = BIO_new(BIO_s_mem());
			if (!bio_read_)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("BIO_new() failed");

				SSL_free(ssl_);
				goto error;
			}
			bio_write_ = BIO_new(BIO_s_mem());
			if (!bio_write_)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("BIO_new() failed");
				BIO_free(bio_write_);
				SSL_free(ssl_);
				goto error;
			}
			SSL_set_bio(ssl_, bio_read_, bio_write_);

			SSL_set_mtu(ssl_, kDtlsMtu);
			DTLS_set_link_mtu(ssl_, kDtlsMtu);
			// Set callback handler for setting DTLS timer interval.
			DTLS_set_timer_cb(ssl_, OnSslDtlsTimer);

			//dtls_queue_.PostDelayedTask([this]() {  /*RTC_DCHECK_RUN_ON(&dtls_queue_);*/ OnTimer(); }, timeoutMs);

			//SSL_set_accept_state(ssl_);
			return  ;

		error:

			// NOTE: At this point SSL_set_bio() was not called so we must free BIOs as
			// well.
			if (bio_read_)
				BIO_free(bio_read_);

			if (bio_write_)
				BIO_free(bio_write_);

			if (this->ssl_)
				SSL_free(this->ssl_);

			// NOTE: If this is not catched by the caller the program will abort, but
			// this should never happen.
			LIBSSL_LOG_T_F(LS_ERROR) << ("DtlsTransport instance creation failed");
			return  ;
		}

		//}
		Dtls::~Dtls()
		{
#if SAMPLE_SSL
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
#else 

			if (IsRunning())
			{
				// Send close alert to the peer.
				SSL_shutdown(this->ssl_);
				SendPendingOutgoingDtlsData();
			}

			if (this->ssl_)
			{
				SSL_free(this->ssl_);

				this->ssl_ = nullptr;
				this->bio_read_ = nullptr;
				this->bio_write_ = nullptr;
			}


#endif //
		}
		void Dtls::Run(Role local_role)
		{
			RTC_CHECK_DESC(
				local_role_ == Role::CLIENT || local_role_ == Role::SERVER,
				"local DTLS role must be 'client' or 'server'");

			Role previousLocalRole = this->local_role_;

			if (local_role == previousLocalRole)
			{
				LIBSSL_LOG_T_F(LS_INFO)<<("same local DTLS role provided, doing nothing");

				return;
			}

			// If the previous local DTLS role was 'client' or 'server' do reset.
			if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER)
			{
				LIBSSL_LOG_T_F(LS_INFO) << "resetting DTLS due to local role change";

				Reset();
			}

			// Update local role.
			this->local_role_ = local_role;

			// Set state and notify the listener.
			this->state_ = DtlsState::CONNECTING;
			//this->listener->OnDtlsTransportConnecting(this);
			SignalDtlsConnecting(this);

			switch (this->local_role_)
			{
				case Role::CLIENT:
				{
					//MS_DEBUG_TAG(dtls, "running [role:client]");
					LIBSSL_LOG(LS_INFO) << "running [role:client]";
					///  ????????????????????????????? dtls ???? 交换协商的流程

					SSL_set_connect_state(this->ssl_);
					SSL_do_handshake(this->ssl_);
					SendPendingOutgoingDtlsData();
					SetTimeout();

					break;
				}

				case Role::SERVER:
				{
					//MS_DEBUG_TAG(dtls, "running [role:server]");
					LIBSSL_LOG(LS_INFO) << "running [role:server]";
					SSL_set_accept_state(this->ssl_);
					SSL_do_handshake(this->ssl_);

					break;
				}

				default:
				{
					//RTC_CHECK(
					//	local_role_ == Role::CLIENT || local_role_ == Role::SERVER,
					//	"local DTLS role must be 'client' or 'server'");
					RTC_FATAL("invalid local DTLS role");
				}
			}
		}
		
		void Dtls::OnRecv(const char *data, int32_t size)
		{
#if SAMPLE_SSL
			BIO_reset(bio_read_);
			BIO_reset(bio_write_);

			BIO_write(bio_read_, data, size);
			SSL_do_handshake(ssl_);

			NeedPost();
 
			
			//if (is_done_)
			//{
			//	GetSrtpKey();
			//	 
			//	SignalDtlsHandshakeDone(this);
			//	return;
			//}
 
			int32_t read = SSL_read(ssl_, buffer_, 65535);
			if (!CheckStatus(read))
			{
				return;
			}

			// application data reccvide  not if 
			// datachannel 
			// Application data received. Notify to the listener.
			if (read > 0)
			{
				// It is allowed to receive DTLS data even before validating remote fingerprint.
				if (!handshake_done_)
				{
					//MS_WARN_TAG(dtls, "ignoring application data received while DTLS handshake not done");
					LIBRTC_LOG(LS_WARNING) << "ignoring application data received while DTLS handshake not done";
					return;
				}

				// Notify the listener.
				//this->listener->OnDtlsTransportApplicationDataReceived(
					//this, (uint8_t*)DtlsTransport::sslReadBuffer, static_cast<size_t>(read));
			}
#else 

			int written;
			int read;

			if (!IsRunning())
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("cannot process data while not running");

				return;
			}

			// Write the received DTLS data into the sslBioFromNetwork.
			written =
				BIO_write(this->bio_read_, static_cast<const void*>(data), static_cast<int>(size));

			if (written != static_cast<int>(size))
			{
				LIBSSL_LOG_T_F(LS_WARNING) << 
					"OpenSSL BIO_write() wrote less ("<< written <<" bytes) than given data ("<< size <<" bytes)" ;
			}

			// Must call SSL_read() to process received DTLS data.
			read = SSL_read(this->ssl_, static_cast<void*>(ssl_read_buffer_), kSsslReadBufferSize);

			// Send data if it's ready.
			SendPendingOutgoingDtlsData();

			// Check SSL status and return if it is bad/closed.
			if (!CheckStatus(read))
				return;

			// Set/update the DTLS timeout.
			if (!SetTimeout())
				return;

			// Application data received. Notify to the listener.
			if (read > 0)
			{
				// It is allowed to receive DTLS data even before validating remote fingerprint.
				if (!this->handshake_done_)
				{
					//MS_WARN_TAG(dtls, "ignoring application data received while DTLS handshake not done");
					LIBSSL_LOG_T_F(LS_WARNING) << "ignoring application data received while DTLS handshake not done";
					return;
				}

				// Notify the listener.
				//this->listener->OnDtlsTransportApplicationDataReceived(
				//	this, (uint8_t*)DtlsTransport::sslReadBuffer, static_cast<size_t>(read));
				SignalDtlsApplicationDataReceived(this, 
					(uint8_t*)ssl_read_buffer_, static_cast<size_t>(read));
			}
#endif //
		}

		bool Dtls::SetRemoteFingerprint(Fingerprint fingerprint)
		{
			RTC_CHECK_DESC(
				fingerprint.algorithm != FingerprintAlgorithm::NONE, "no fingerprint algorithm provided");

			this->remote_fingerprint_ = fingerprint;

			// The remote fingerpring may have been set after DTLS handshake was done,
			// so we may need to process it now.
			if (this->handshake_done_ && this->state_ != DtlsState::CONNECTED)
			{
				LIBSSL_LOG(LS_INFO)<< "handshake already done, processing it right now";

				return ProcessHandshake();
			}

			return true;
		}

		bool Dtls::CheckStatus(int returnCode)
		{
			int err;
			bool wasHandshakeDone = handshake_done_;

			err = SSL_get_error(ssl_, returnCode);

			switch (err)
			{
			case SSL_ERROR_NONE:
				break;

			case SSL_ERROR_SSL:
				//LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SSL");
				LIBRTC_LOG(LS_ERROR) << "SSL status: SSL_ERROR_SSL";
				break;

			case SSL_ERROR_WANT_READ:
				break;

			case SSL_ERROR_WANT_WRITE:
				//MS_WARN_TAG(dtls, "SSL status: SSL_ERROR_WANT_WRITE");
				LIBRTC_LOG(LS_WARNING) << "SSL status: SSL_ERROR_WANT_WRITE";
				break;

			case SSL_ERROR_WANT_X509_LOOKUP:
				LIBRTC_LOG(LS_ERROR) << "SSL status: SSL_ERROR_WANT_X509_LOOKUP";
				break;

			case SSL_ERROR_SYSCALL:
				LIBRTC_LOG(LS_WARNING) << ("SSL status: SSL_ERROR_SYSCALL");
				break;

			case SSL_ERROR_ZERO_RETURN:
				break;

			case SSL_ERROR_WANT_CONNECT:
				LIBRTC_LOG(LS_WARNING) << "SSL status: SSL_ERROR_WANT_CONNECT";
				break;

			case SSL_ERROR_WANT_ACCEPT:
				LIBRTC_LOG(LS_WARNING) << "SSL status: SSL_ERROR_WANT_ACCEPT";
				break;

			default:
				LIBRTC_LOG(LS_WARNING) << "SSL status: unknown error";
			}

			// Check if the handshake (or re-handshake) has been done right now.
			if (handshake_done_now_)
			{
				handshake_done_now_ = false;
				handshake_done_ = true;
			
				// Stop the timer.
				//this->timer->Stop();
			
				// Process the handshake just once (ignore if DTLS renegotiation).
				if (!wasHandshakeDone && this->remote_fingerprint_.algorithm != FingerprintAlgorithm::NONE)
				{
					return ProcessHandshake();
				} 
				return true;
			}
			// Check if the peer sent close alert or a fatal error happened.
			else if (((SSL_get_shutdown(ssl_) & SSL_RECEIVED_SHUTDOWN) != 0) || err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL)
			{ 
				if ( this->state_ == DtlsState::CONNECTED)
				{
					//MS_DEBUG_TAG(dtls, "disconnected");
					LIBRTC_LOG(LS_INFO) << "disconnected";
					 Reset();
					handshake_done_ = false;
					// Set state and notify the listener.
					 this->state_ = DtlsState::CLOSED;
					//this->listener->OnDtlsTransportClosed(this);
					SignalDtlsClose(this);
				}
				else
				{
					LIBRTC_LOG(LS_INFO) << "connection failed";
					//MS_WARN_TAG(dtls, "connection failed");

					 Reset();

					// Set state and notify the listener.
					 this->state_ = DtlsState::FAILED;
					 SignalDtlsFailed(this);
					//this->listener->OnDtlsTransportFailed(this);
				}

				
				return false;
			} 
			else
			{
				return true;
			}
		}
		
		
		 
	
		
		 
		
		

		void Dtls::OnSslInfo(int32_t where, int32_t ret)
		{
			//Dtls *dtls = static_cast<Dtls*>(SSL_get_ex_data(ssl, 0));
			int w = where & ~SSL_ST_MASK;
			const char* role;
			if (w & SSL_ST_CONNECT)
			{
				//dtls->SetClient(true);
				//SetClient(true);
				role = "client";
			}
			else if (w & SSL_ST_ACCEPT)
			{
				// SetClient(false);
				role = "server";
			}
			else
			{
				// SetClient(false);
				role = "undefined";
			}
			if ((where & SSL_CB_LOOP) != 0)
			{
				LIBRTC_LOG(LS_INFO) << "[role: " << role << ", action:'" << SSL_state_string_long(ssl_) << "']";
				//MS_DEBUG_TAG(dtls, "[role:%s, action:'%s']", role, SSL_state_string_long(ssl));
			}
			else if ((where & SSL_CB_ALERT) != 0)
			{
				const char* alertType;

				switch (*SSL_alert_type_string(ret))
				{
				case 'W':
					alertType = "warning";
					break;

				case 'F':
					alertType = "fatal";
					break;

				default:
					alertType = "undefined";
				}

				if ((where & SSL_CB_READ) != 0)
				{
					//MS_WARN_TAG(dtls, "received DTLS %s alert: %s", alertType, SSL_alert_desc_string_long(ret));
					LIBRTC_LOG(LS_INFO) << "[received DTLS  " << alertType << ", alert:'" << SSL_alert_desc_string_long(ret) << "']";
				}
				else if ((where & SSL_CB_WRITE) != 0)
				{
					//MS_DEBUG_TAG(dtls, "sending DTLS %s alert: %s", alertType, SSL_alert_desc_string_long(ret));
					LIBRTC_LOG(LS_INFO) << "[sending DTLS  " << alertType << ", alert:'" << SSL_alert_desc_string_long(ret) << "']";
				}
				else
				{
					//MS_DEBUG_TAG(dtls, "DTLS %s alert: %s", alertType, SSL_alert_desc_string_long(ret));
					LIBRTC_LOG(LS_INFO) << "[  DTLS  " << alertType << ", alert:'" << SSL_alert_desc_string_long(ret) << "']";
				}
			}
			else if ((where & SSL_CB_EXIT) != 0)
			{
				if (ret == 0)
				{
					//MS_DEBUG_TAG(dtls, "[role:%s, failed:'%s']", role, SSL_state_string_long(this->ssl));
					LIBRTC_LOG(LS_INFO) << "[  role:  " << role << ", failed:'" << SSL_state_string_long(ssl_) << "']";
				}
				else if (ret < 0)
				{
					//MS_DEBUG_TAG(dtls, "role: %s, waiting:'%s']", role, SSL_state_string_long(this->ssl));
					LIBRTC_LOG(LS_INFO) << "[  role:  " << role << ", waiting:'" << SSL_state_string_long(ssl_) << "']";
				}
			}
			else if ((where & SSL_CB_HANDSHAKE_START) != 0)
			{
				//MS_DEBUG_TAG(dtls, "DTLS handshake start");
				LIBRTC_LOG(LS_INFO) << "DTLS handshake start";
			}
			else if ((where & SSL_CB_HANDSHAKE_DONE) != 0)
			{
				//MS_DEBUG_TAG(dtls, "DTLS handshake done");
				LIBRTC_LOG(LS_INFO) << "DTLS handshake done";
				 //SetDone();
				//this->handshakeDoneNow = true;
				handshake_done_now_ = true;
			}
		}

		void Dtls::OnTimer()
		{
			// Workaround for https://github.com/openssl/openssl/issues/7998.
			if (this->handshake_done_)
			{
				LIBSSL_LOG_T_F(LS_ERROR) << ("handshake is done so return");

				return;
			}

			DTLSv1_handle_timeout(this->ssl_);

			// If required, send DTLS data.
			SendPendingOutgoingDtlsData();

			// Set the DTLS timer again.
			SetTimeout();
		}

		void Dtls::SendPendingOutgoingDtlsData()
		{
			if (BIO_eof(this->bio_write_))
			{
				return;
			}

			int64_t read;
			char* data{ nullptr };

			read = BIO_get_mem_data(this->bio_write_, &data); // NOLINT

			if (read <= 0)
				return;

			//MS_DEBUG_DEV("%" PRIu64 " bytes of DTLS data ready to sent to the peer", read);
			LIBSSL_LOG(LS_INFO) << read << " bytes of DTLS data ready to sent to the peer";
			// Notify the listener.

			// send data 
			//this->listener->OnDtlsTransportSendData(
			//	this, reinterpret_cast<uint8_t*>(data), static_cast<size_t>(read));
			//const char *, size_t, Dtls*
			SignalDtlsSendPakcet(this, reinterpret_cast<const uint8_t *>(data), static_cast<size_t>(read));
			// Clear the BIO buffer.
			// NOTE: the (void) avoids the -Wunused-value warning.
			(void)BIO_reset(this->bio_write_);
		}

		bool Dtls::SetTimeout()
		{
			RTC_CHECK(
				this->state_ == DtlsState::CONNECTING || this->state_ == DtlsState::CONNECTED,
				"invalid DTLS state");

			int64_t ret;
		 	//uv_timeval_t dtlsTimeout{ 0, 0 };
			struct  timeval dtlsTimeout { 0, 0 } ;
			uint64_t timeoutMs;

			// NOTE: If ret == 0 then ignore the value in dtlsTimeout.
			// NOTE: No DTLSv_1_2_get_timeout() or DTLS_get_timeout() in OpenSSL 1.1.0-dev.
			ret = DTLSv1_get_timeout(this->ssl_, static_cast<void*>(&dtlsTimeout)); // NOLINT

			if (ret == 0)
			{
				return true;
			}

			timeoutMs = (dtlsTimeout.tv_sec * static_cast<uint64_t>(1000)) + (dtlsTimeout.tv_usec / 1000);

			if (timeoutMs == 0)
			{
				return true;
			}
			else if (timeoutMs < 30000)
			{
				//MS_DEBUG_DEV("DTLS timer set in %" PRIu64 "ms", timeoutMs);

				//this->timer->Start(timeoutMs);
				LIBSSL_LOG(LS_INFO) << "DTLS timer set in  "  << timeoutMs << "ms";
				/*
				[this]() {
            RTC_DCHECK_RUN_ON(&task_queue_);
            if (!Process()) {
              next_process_ms_.reset();
            }
          }
				*/
				dtls_queue_.PostDelayedTask( [this]() {  /*RTC_DCHECK_RUN_ON(&dtls_queue_);*/ OnTimer(); }, timeoutMs);
				return true;
			}
			// NOTE: Don't start the timer again if the timeout is greater than 30 seconds.
			else
			{
				LIBSSL_LOG(LS_INFO) << "DTLS timeout too high ( " << timeoutMs << "ms), resetting DLTS" ;

				Reset();

				// Set state and notify the listener.
				this->state_ = DtlsState::FAILED;
				//this->listener->OnDtlsTransportFailed(this);
				SignalDtlsFailed(this);
				return false;
			}
		}

		void Dtls::Reset()
		{
			int ret;

			if (!IsRunning())
			{
				return;
			}

			//MS_WARN_TAG(dtls, "resetting DTLS transport");
			LIBSSL_LOG(LS_INFO) << "resetting DTLS transport";
			// Stop the DTLS timer.
			//this->timer->Stop();
			//dtls_queue_.~TaskQueue();
			// We need to reset the SSL instance so we need to "shutdown" it, but we
			// don't want to send a Close Alert to the peer, so just don't call
			// SendPendingOutgoingDTLSData().
			SSL_shutdown(this->ssl_);

			this->local_role_ = Role::NONE;
			this->state_ = DtlsState::NONE;
			this->handshake_done_ = false;
			this->handshake_done_now_ = false;

			// Reset SSL status.
			// NOTE: For this to properly work, SSL_shutdown() must be called before.
			// NOTE: This may fail if not enough DTLS handshake data has been received,
			// but we don't care so just clear the error queue.
			ret = SSL_clear(this->ssl_);

			if (ret == 0)
			{
				ERR_clear_error();
			}
		}
		bool Dtls::ProcessHandshake()
		{
			RTC_CHECK_DESC(this->handshake_done_, "handshake not done yet");
			RTC_CHECK_DESC(
				this->remote_fingerprint_.algorithm != FingerprintAlgorithm::NONE, "remote fingerprint not set");

			// Validate the remote fingerprint.
			if (!CheckRemoteFingerprint())
			{
				Reset();

				// Set state and notify the listener.
				this->state_ = DtlsState::FAILED;
				//this->listener->OnDtlsTransportFailed(this);
				SignalDtlsFailed(this);
				return false;
			}

			// Get the negotiated SRTP crypto suite.
			libsrtp::CryptoSuite srtpCryptoSuite = GetNegotiatedSrtpCryptoSuite();

			if (srtpCryptoSuite != libsrtp::CryptoSuite::NONE)
			{
				// Extract the SRTP keys (will notify the listener with them).
				ExtractSrtpKeys(srtpCryptoSuite);

				return true;
			}

			// NOTE: We assume that "use_srtp" DTLS extension is required even if
			// there is no audio/video.
			//MS_WARN_2TAGS(dtls, srtp, "SRTP crypto suite not negotiated");
			LIBSRTP_LOG(LS_INFO) << "SRTP crypto suite not negotiated";
			Reset();

			// Set state and notify the listener.
			this->state_ = DtlsState::FAILED;
			//this->listener->OnDtlsTransportFailed(this);
			SignalDtlsFailed(this);
			return false;
		}

		bool Dtls::CheckRemoteFingerprint()
		{
			RTC_CHECK_DESC(
				this->remote_fingerprint_.algorithm != FingerprintAlgorithm::NONE, "remote fingerprint not set");

			X509* certificate;
			uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
			unsigned int size{ 0 };
			char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
			const EVP_MD* hashFunction;
			int ret;

			certificate = SSL_get_peer_certificate(this->ssl_);

			if (!certificate)
			{
				//MS_WARN_TAG(dtls, "no certificate was provided by the peer");
				LIBSSL_LOG_T_F(LS_WARNING) << "no certificate was provided by the peer";
				return false;
			}

			switch (remote_fingerprint_.algorithm)
			{
			case FingerprintAlgorithm::SHA1:
				hashFunction = EVP_sha1();
				break;

			case FingerprintAlgorithm::SHA224:
				hashFunction = EVP_sha224();
				break;

			case FingerprintAlgorithm::SHA256:
				hashFunction = EVP_sha256();
				break;

			case FingerprintAlgorithm::SHA384:
				hashFunction = EVP_sha384();
				break;

			case FingerprintAlgorithm::SHA512:
				hashFunction = EVP_sha512();
				break;

			default:
				RTC_CHECK_DESC(false, "unknown algorithm");
			}

			// Compare the remote fingerprint with the value given via signaling.
			ret = X509_digest(certificate, hashFunction, binaryFingerprint, &size);

			if (ret == 0)
			{
				RTC_CHECK_DESC(false, "X509_digest() failed");

				X509_free(certificate);

				return false;
			}

			// Convert to hexadecimal format in uppercase with colons.
			for (unsigned int i{ 0 }; i < size; ++i)
			{
				std::sprintf(hexFingerprint + (i * 3), "%.2X:", binaryFingerprint[i]);
			}
			hexFingerprint[(size * 3) - 1] = '\0';

			if ( remote_fingerprint_.value != hexFingerprint)
			{
				LIBSSL_LOG_T_F(LS_WARNING) << "fingerprint in the remote certificate (" << hexFingerprint
					<< ") does not match the announced one ("<< remote_fingerprint_.value <<")";
					  
				X509_free(certificate);

				return false;
			}

			//MS_DEBUG_TAG(dtls, "valid remote fingerprint");
			LIBSSL_LOG(LS_INFO) << "valid remote fingerprint";
			// Get the remote certificate in PEM format.

			BIO* bio = BIO_new(BIO_s_mem());

			// Ensure the underlying BUF_MEM structure is also freed.
			// NOTE: Avoid stupid "warning: value computed is not used [-Wunused-value]" since
			// BIO_set_close() always returns 1.
			(void)BIO_set_close(bio, BIO_CLOSE);

			ret = PEM_write_bio_X509(bio, certificate);

			if (ret != 1)
			{
				LIBSSL_LOG(LS_ERROR) << ("PEM_write_bio_X509() failed");

				X509_free(certificate);
				BIO_free(bio);

				return false;
			}

			BUF_MEM* mem;

			BIO_get_mem_ptr(bio, &mem); // NOLINT[cppcoreguidelines-pro-type-cstyle-cast]

			if (!mem || !mem->data || mem->length == 0u)
			{
				LIBSSL_LOG(LS_ERROR) << ("BIO_get_mem_ptr() failed");

				X509_free(certificate);
				BIO_free(bio);

				return false;
			}

			 remote_cert_ = std::string(mem->data, mem->length);

			X509_free(certificate);
			BIO_free(bio);

			return true;
		}
		void Dtls::ExtractSrtpKeys(libsrtp::CryptoSuite srtpCryptoSuite)
		{
			size_t srtpKeyLength{ 0 };
			size_t srtpSaltLength{ 0 };
			size_t srtpMasterLength{ 0 };

			switch (srtpCryptoSuite)
			{
			case libsrtp::CryptoSuite::AES_CM_128_HMAC_SHA1_80:
			case libsrtp::CryptoSuite::AES_CM_128_HMAC_SHA1_32:
			{
				srtpKeyLength = libsrtp::kSrtpMasterKeyLength;
				srtpSaltLength = libsrtp::kSrtpMasterSaltLength;
				srtpMasterLength = libsrtp::kSrtpMasterLength;

				break;
			}

			case libsrtp::CryptoSuite::AEAD_AES_256_GCM:
			{
				srtpKeyLength = libsrtp::kSrtpAesGcm256MasterKeyLength;
				srtpSaltLength = libsrtp::kSrtpAesGcm256MasterSaltLength;
				srtpMasterLength = libsrtp::kSrtpAesGcm256MasterLength;

				break;
			}

			case libsrtp::CryptoSuite::AEAD_AES_128_GCM:
			{
				srtpKeyLength = libsrtp::kSrtpAesGcm128MasterKeyLength;
				srtpSaltLength = libsrtp::kSrtpAesGcm128MasterSaltLength;
				srtpMasterLength = libsrtp::kSrtpAesGcm128MasterLength;

				break;
			}

			default:
			{
				RTC_CHECK_DESC(false, "unknown SRTP crypto suite");
			}
			}

			auto* srtpMaterial = new uint8_t[srtpMasterLength * 2];
			uint8_t* srtpLocalKey{ nullptr };
			uint8_t* srtpLocalSalt{ nullptr };
			uint8_t* srtpRemoteKey{ nullptr };
			uint8_t* srtpRemoteSalt{ nullptr };
			auto* srtpLocalMasterKey = new uint8_t[srtpMasterLength];
			auto* srtpRemoteMasterKey = new uint8_t[srtpMasterLength];
			int ret;

			ret = SSL_export_keying_material(
				this->ssl_, srtpMaterial, srtpMasterLength * 2, "EXTRACTOR-dtls_srtp", 19, nullptr, 0, 0);

			RTC_CHECK_DESC(ret != 0, "SSL_export_keying_material() failed");

			switch ( local_role_)
			{
			case Role::SERVER:
			{
				srtpRemoteKey = srtpMaterial;
				srtpLocalKey = srtpRemoteKey + srtpKeyLength;
				srtpRemoteSalt = srtpLocalKey + srtpKeyLength;
				srtpLocalSalt = srtpRemoteSalt + srtpSaltLength;

				break;
			}

			case Role::CLIENT:
			{
				srtpLocalKey = srtpMaterial;
				srtpRemoteKey = srtpLocalKey + srtpKeyLength;
				srtpLocalSalt = srtpRemoteKey + srtpKeyLength;
				srtpRemoteSalt = srtpLocalSalt + srtpSaltLength;

				break;
			}

			default:
			{
				RTC_CHECK_DESC(false, "no DTLS role set");
			}
			}

			// Create the SRTP local master key.
			std::memcpy(srtpLocalMasterKey, srtpLocalKey, srtpKeyLength);
			std::memcpy(srtpLocalMasterKey + srtpKeyLength, srtpLocalSalt, srtpSaltLength);
			// Create the SRTP remote master key.
			std::memcpy(srtpRemoteMasterKey, srtpRemoteKey, srtpKeyLength);
			std::memcpy(srtpRemoteMasterKey + srtpKeyLength, srtpRemoteSalt, srtpSaltLength);

			// Set state and notify the listener.
			this->state_ = DtlsState::CONNECTED;
			//this->listener->OnDtlsTransportConnected(
			//	this,
			//	srtpCryptoSuite,
			//	srtpLocalMasterKey,
			//	srtpMasterLength,
			//	srtpRemoteMasterKey,
			//	srtpMasterLength,
			//	this->remoteCert);
			SignalDtlsConnected(this,  
				srtpCryptoSuite,
				srtpLocalMasterKey,
				srtpMasterLength,
				srtpRemoteMasterKey,
				srtpMasterLength,
				remote_cert_);






			delete[] srtpMaterial;
			delete[] srtpLocalMasterKey;
			delete[] srtpRemoteMasterKey;
		}
		libsrtp::CryptoSuite  Dtls::GetNegotiatedSrtpCryptoSuite()
		{
			libsrtp::CryptoSuite negotiatedSrtpCryptoSuite = libsrtp::CryptoSuite::NONE;

			// Ensure that the SRTP crypto suite has been negotiated.
			// NOTE: This is a OpenSSL type.
			SRTP_PROTECTION_PROFILE* sslSrtpCryptoSuite = SSL_get_selected_srtp_profile(this->ssl_);

			if (!sslSrtpCryptoSuite)
				return negotiatedSrtpCryptoSuite;

			// Get the negotiated SRTP crypto suite.
			for (auto& srtpCryptoSuite : libsrtp::kSrtpCryptoSuites)
			{
				libsrtp::SrtpCryptoSuiteMapEntry* cryptoSuiteEntry = std::addressof(srtpCryptoSuite);

				if (std::strcmp(sslSrtpCryptoSuite->name, cryptoSuiteEntry->name) == 0)
				{
					//MS_DEBUG_2TAGS(dtls, srtp, "chosen SRTP crypto suite: %s", cryptoSuiteEntry->name);
					LIBSRTP_LOG(LS_INFO) << "chosen SRTP crypto suite:" << cryptoSuiteEntry->name;
					negotiatedSrtpCryptoSuite = cryptoSuiteEntry->crypto_suite;
				}
			}

			RTC_CHECK_DESC(
				negotiatedSrtpCryptoSuite != libsrtp::CryptoSuite::NONE,
				"chosen SRTP crypto suite is not an available one");

			return negotiatedSrtpCryptoSuite;
		}
	}

}

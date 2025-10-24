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


#ifndef _C_LIBRTC_DTLS__H_
#define _C_LIBRTC_DTLS__H_

#include <cstddef>

#include "libmedia_transfer_protocol/librtc/dtls_certs.h"


namespace libmedia_transfer_protocol {

	namespace libssl
	{
		static const int32_t  kDtlsMtu{ 1350 };
		static const int32_t  kSsslReadBufferSize{ 65536 };
		enum  class  DtlsState
		{
			NONE = 1,
			CONNECTING,
			CONNECTED,
			FAILED,
			CLOSED
		};
		 
		enum class  Role
		{
			NONE = 0,
			AUTO = 1,
			CLIENT,
			SERVER
		};
		


		class Dtls : public sigslot::has_slots<>
		{
		public:
			Dtls(webrtc::TaskQueueFactory* task_queue_factory)  ;
			~Dtls();


			void Run(Role local_role);

			 
			void OnRecv(const uint8_t *data, int32_t size);
			bool SetRemoteFingerprint(Fingerprint fingerprint);
			 
			

		public:

			
			// alter

			sigslot::signal1<Dtls*>									SignalDtlsConnecting;
			sigslot::signal7<Dtls*, libsrtp::CryptoSuite  ,
				uint8_t*  , size_t  ,
				uint8_t*  , size_t  , std::string&  >				SignalDtlsConnected; 
			sigslot::signal1<Dtls*>									SignalDtlsClose;
			sigslot::signal1<Dtls*>									SignalDtlsFailed; 
			sigslot::signal3< Dtls*,const uint8_t *, size_t>		SignalDtlsSendPakcet; 
			sigslot::signal3<  Dtls*, const uint8_t *, size_t>		SignalDtlsApplicationDataReceived;
		private:
			 
			 
			 


		public:
			/* Callbacks fired by OpenSSL events. */
			void OnSslInfo(int32_t where, int32_t ret);

			void OnTimer();

		private:
			bool IsRunning() const
			{
				switch (this->state_)
				{
				case DtlsState::NONE:
					return false;
				case DtlsState::CONNECTING:
				case DtlsState::CONNECTED:
					return true;
				case DtlsState::FAILED:
				case DtlsState::CLOSED:
					return false;
				}

				// Make GCC 4.9 happy.
				return false;
			}

			
			void Reset();
			bool CheckStatus(int returnCode);
			void SendPendingOutgoingDtlsData();
			bool SetTimeout();
			bool ProcessHandshake();
			bool CheckRemoteFingerprint();
			void ExtractSrtpKeys(libsrtp::CryptoSuite srtpCryptoSuite);
			libmedia_transfer_protocol::libsrtp::CryptoSuite GetNegotiatedSrtpCryptoSuite();

			
		private: 
		private: 
			//std::string send_key_;
			//std::string recv_key_;


			///
			///
			// Others 
			SSL * ssl_{ nullptr };
			BIO * bio_read_{ nullptr };
			BIO * bio_write_{ nullptr };
			uint8_t   ssl_read_buffer_[kSsslReadBufferSize];
			DtlsState  state_{ DtlsState::NONE };
			Role    local_role_{ Role::NONE };

			Fingerprint remote_fingerprint_;
			
			//libssl::Fingerprint    remote_finger_print_;
			//bool handshake_done_{ false };
			bool handshake_done_{ false };
			bool handshake_done_now_{ false };
			std::string remote_cert_;
			
			

			 //
			 rtc::TaskQueue   dtls_queue_;
			 
		};
	}

}

#endif //
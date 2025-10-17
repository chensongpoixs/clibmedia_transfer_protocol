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
				   date:  2025-10-08

GB28181使用RTP传输音视频，有两种方式：UDP、TCP。UDP和RTSP中的没有区别，但是TCP有区别。

		目前RTSP有两个版本1.0和2.0，1.0定义在RFC2326中，2.0定义在RFC7826。2.0是2016年由IETF发布的RTSP新标准，不过现在基本使用的都是RTSP1.0，就算有使用2.0的，也会兼容1.0。
		而GB28181则使用RFC4571中定义的RTP，这里面RTP over TCP方式和以往的不同。

		RFC2326中RTP over TCP的数据包是这样的：

| magic number | channel number | data length | data  |magic number -

magic number：   RTP数据标识符，"$" 一个字节
channel number： 信道数字 - 1个字节，用来指示信道
data length ：   数据长度 - 2个字节，用来指示插入数据长度
data ：          数据 - ，比如说RTP包，总长度与上面的数据长度相同
		而RFC4571中的RTP over TCP的数据包确是这样的：

	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	---------------------------------------------------------------
   |             LENGTH            |  RTP or RTCP packet ...       |
	---------------------------------------------------------------
		RFC2326中用channel number标识消息类型，因为RTSP中信令和和音视频都是通过同一个TCP通道传输，所以必须通过channel number区分。而GB28181中信令和媒体数据是不同的传输通道，所以不用去区分。

		RFC4571标准格式：长度(2字节) + RTP头 + 数据

		RFC2326标准格式：$(1字节) + 通道号(1字节) + 长度(2字节) + RTP头 + 数据

 ******************************************************************************/

 
#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "libmedia_transfer_protocol/libhttp/http_type.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"
#include <string>
#include <unordered_map>
#include <iostream>
 //#include <ctype.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <cassert>


#include <algorithm>


#include <assert.h>
#include <cstdalign>
#include <cstdint>
#include <cstdio>
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
//#include <winsock.h>
#include <WinSock2.h>
#include <Windows.h>
#elif defined(__GNUC__)
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#else
// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö§ï¿½ÖµÄ±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½Ô¼ï¿½Êµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
#error unexpected c complier (msc/gcc), Need to implement this method for demangle
#endif
 

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{


		namespace  
		{
			static constexpr size_t kBufferOffset{ 8 };
			static constexpr char CRLF[]{ "\r\n" };
		}

		MsgBuffer::MsgBuffer(size_t len)
			: head_(kBufferOffset), initCap_(len), buffer_(len + head_), tail_(head_)
		{
		}

		void MsgBuffer::EnsureWritableBytes(size_t len)
		{
			if (WritableBytes() >= len)
				return;
			if (head_ + WritableBytes() >=
				(len + kBufferOffset))  // move Readable bytes
			{
				std::copy(begin() + head_, begin() + tail_, begin() + kBufferOffset);
				tail_ = kBufferOffset + (tail_ - head_);
				head_ = kBufferOffset;
				return;
			}
			// create new buffer
			size_t newLen;
			if ((buffer_.size() * 2) > (kBufferOffset + ReadableBytes() + len))
				newLen = buffer_.size() * 2;
			else
				newLen = kBufferOffset + ReadableBytes() + len;
			MsgBuffer newbuffer(newLen);
			newbuffer.Append(*this);
			Swap(newbuffer);
		}
		void MsgBuffer::Swap(MsgBuffer &buf) noexcept
		{
			buffer_.swap(buf.buffer_);
			std::swap(head_, buf.head_);
			std::swap(tail_, buf.tail_);
			std::swap(initCap_, buf.initCap_);
		}
		void MsgBuffer::Append(const MsgBuffer &buf)
		{
			EnsureWritableBytes(buf.ReadableBytes());
			memcpy(&buffer_[tail_], buf.Peek(), buf.ReadableBytes());
			tail_ += buf.ReadableBytes();
		}
		void MsgBuffer::Append(const char *buf, size_t len)
		{
			EnsureWritableBytes(len);
			memcpy(&buffer_[tail_], buf, len);
			tail_ += len;
		}
		void MsgBuffer::AppendInt16(const uint16_t s)
		{
			uint16_t ss = htons(s);
			Append(static_cast<const char *>((void *)&ss), 2);
		}
		void MsgBuffer::AppendInt32(const uint32_t i)
		{
			uint32_t ii = htonl(i);
			Append(static_cast<const char *>((void *)&ii), 4);
		}
		void MsgBuffer::AppendInt64(const uint64_t l)
		{
			uint64_t ll = hton64(l);
			Append(static_cast<const char *>((void *)&ll), 8);
		}

		void MsgBuffer::AddInFrontInt16(const uint16_t s)
		{
			uint16_t ss = htons(s);
			AddInFront(static_cast<const char *>((void *)&ss), 2);
		}
		void MsgBuffer::AddInFrontInt32(const uint32_t i)
		{
			uint32_t ii = htonl(i);
			AddInFront(static_cast<const char *>((void *)&ii), 4);
		}
		void MsgBuffer::AddInFrontInt64(const uint64_t l)
		{
			uint64_t ll = hton64(l);
			AddInFront(static_cast<const char *>((void *)&ll), 8);
		}

		uint16_t MsgBuffer::PeekInt16() const
		{
			assert(ReadableBytes() >= 2);
			uint16_t rs = *(static_cast<const uint16_t *>((void *)Peek()));
			return ntohs(rs);
		}
		uint32_t MsgBuffer::PeekInt32() const
		{
			assert(ReadableBytes() >= 4);
			uint32_t rl = *(static_cast<const uint32_t *>((void *)Peek()));
			return ntohl(rl);
		}
		uint64_t MsgBuffer::PeekInt64() const
		{
			assert(ReadableBytes() >= 8);
			uint64_t rll = *(static_cast<const uint64_t *>((void *)Peek()));
			return ntoh64(rll);
		}

		void MsgBuffer::Retrieve(size_t len)
		{
			if (len >= ReadableBytes())
			{
				RetrieveAll();
				return;
			}
			head_ += len;
		}
		void MsgBuffer::RetrieveAll()
		{
			if (buffer_.size() > (initCap_ * 2))
			{
				buffer_.resize(initCap_);
			}
			tail_ = head_ = kBufferOffset;
		}
		//int32_t MsgBuffer::ReadFd(int fd, int *retErrno)
		//{
		//	char extBuffer[8192];
		//	struct iovec vec[2];
		//	size_t writable = WritableBytes();
		//	vec[0].iov_base = begin() + tail_;
		//	vec[0].iov_len = static_cast<int>(writable);
		//	vec[1].iov_base = extBuffer;
		//	vec[1].iov_len = sizeof(extBuffer);
		//	const int iovcnt = (writable < sizeof extBuffer) ? 2 : 1;
		//	ssize_t n = ::readv(fd, vec, iovcnt);
		//	if (n < 0)
		//	{
		//		*retErrno = errno;
		//	}
		//	else if (static_cast<size_t>(n) <= writable)
		//	{
		//		tail_ += n;
		//	}
		//	else
		//	{
		//		tail_ = buffer_.size();
		//		Append(extBuffer, n - writable);
		//	}
		//	return n;
		//}

		std::string MsgBuffer::Read(size_t len)
		{
			if (len > ReadableBytes())
				len = ReadableBytes();
			std::string ret(Peek(), len);
			Retrieve(len);
			return ret;
		}
		uint8_t MsgBuffer::ReadInt8()
		{
			uint8_t ret = PeekInt8();
			Retrieve(1);
			return ret;
		}
		uint16_t MsgBuffer::ReadInt16()
		{
			uint16_t ret = PeekInt16();
			Retrieve(2);
			return ret;
		}
		uint32_t MsgBuffer::ReadInt32()
		{
			uint32_t ret = PeekInt32();
			Retrieve(4);
			return ret;
		}
		uint64_t MsgBuffer::ReadInt64()
		{
			uint64_t ret = PeekInt64();
			Retrieve(8);
			return ret;
		}
		const char *MsgBuffer::FindCRLF() const
		{
			const char *crlf = std::search(Peek(), BeginWrite(), CRLF, CRLF + 2);
			return crlf == BeginWrite() ? NULL : crlf;
		}

		void MsgBuffer::AddInFront(const char *buf, size_t len)
		{
			if (head_ >= len)
			{
				memcpy(begin() + head_ - len, buf, len);
				head_ -= len;
				return;
			}
			if (len <= WritableBytes())
			{
				std::copy(begin() + head_, begin() + tail_, begin() + head_ + len);
				memcpy(begin() + head_, buf, len);
				tail_ += len;
				return;
			}
			size_t newLen;
			if (len + ReadableBytes() < initCap_)
				newLen = initCap_;
			else
				newLen = len + ReadableBytes();
			MsgBuffer newBuf(newLen);
			newBuf.Append(buf, len);
			newBuf.Append(*this);
			Swap(newBuf);
		}

	}
}
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

#ifndef _C_LIBHTTP_MSG_BUFFER_H_
#define _C_LIBHTTP_MSG_BUFFER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libmedia_transfer_protocol/libhttp/http_type.h"
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

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		static constexpr size_t kBufferDefaultLength{ 2048 };
		

		/**
		 * @brief This class represents a memory buffer used for sending and receiving
		 * data.
		 *
		 */
		class MsgBuffer
		{
		public:
			/**
			 * @brief Construct a new message buffer instance.
			 *
			 * @param len The initial size of the buffer.
			 */
			MsgBuffer(size_t len = kBufferDefaultLength);

			/**
			 * @brief Get the beginning of the buffer.
			 *
			 * @return const char*
			 */
			const char *Peek() const
			{
				return begin() + head_;
			}

			/**
			 * @brief Get the end of the buffer where new data can be written.
			 *
			 * @return const char*
			 */
			const char *BeginWrite() const
			{
				return begin() + tail_;
			}
			char *BeginWrite()
			{
				return begin() + tail_;
			}

			/**
			 * @brief Get a byte value from the buffer.
			 *
			 * @return uint8_t
			 */
			uint8_t PeekInt8() const
			{
				assert(ReadableBytes() >= 1);
				return *(static_cast<const uint8_t *>((void *)Peek()));
			}

			/**
			 * @brief Get a unsigned short value from the buffer.
			 *
			 * @return uint16_t
			 */
			uint16_t PeekInt16() const;

			/**
			 * @brief Get a unsigned int value from the buffer.
			 *
			 * @return uint32_t
			 */
			uint32_t PeekInt32() const;

			/**
			 * @brief Get a unsigned int64 value from the buffer.
			 *
			 * @return uint64_t
			 */
			uint64_t PeekInt64() const;

			/**
			 * @brief Get and remove some bytes from the buffer.
			 *
			 * @param len
			 * @return std::string
			 */
			std::string Read(size_t len);

			/**
			 * @brief Get the remove a byte value from the buffer.
			 *
			 * @return uint8_t
			 */
			uint8_t ReadInt8();

			/**
			 * @brief Get and remove a unsigned short value from the buffer.
			 *
			 * @return uint16_t
			 */
			uint16_t ReadInt16();

			/**
			 * @brief Get and remove a unsigned int value from the buffer.
			 *
			 * @return uint32_t
			 */
			uint32_t ReadInt32();

			/**
			 * @brief Get and remove a unsigned int64 value from the buffer.
			 *
			 * @return uint64_t
			 */
			uint64_t ReadInt64();

			/**
			 * @brief swap the buffer with another.
			 *
			 * @param buf
			 */
			void Swap(MsgBuffer &buf) noexcept;

			/**
			 * @brief Return the size of the data in the buffer.
			 *
			 * @return size_t
			 */
			size_t ReadableBytes() const
			{
				return tail_ - head_;
			}

			/**
			 * @brief Return the size of the empty part in the buffer
			 *
			 * @return size_t
			 */
			size_t WritableBytes() const
			{
				return buffer_.size() - tail_;
			}

			/**
			 * @brief Append new data to the buffer.
			 *
			 */
			void Append(const MsgBuffer &buf);
			template <int N>
			void Append(const char(&buf)[N])
			{
				assert(strnlen(buf, N) == N - 1);
				Append(buf, N - 1);
			}
			void Append(const char *buf, size_t len);
			void Append(const std::string &buf)
			{
				Append(buf.c_str(), buf.length());
			}

			/**
			 * @brief Append a byte value to the end of the buffer.
			 *
			 * @param b
			 */
			void AppendInt8(const uint8_t b)
			{
				Append(static_cast<const char *>((void *)&b), 1);
			}

			/**
			 * @brief Append a unsigned short value to the end of the buffer.
			 *
			 * @param s
			 */
			void AppendInt16(const uint16_t s);

			/**
			 * @brief Append a unsigned int value to the end of the buffer.
			 *
			 * @param i
			 */
			void AppendInt32(const uint32_t i);

			/**
			 * @brief Appaend a unsigned int64 value to the end of the buffer.
			 *
			 * @param l
			 */
			void AppendInt64(const uint64_t l);

			/**
			 * @brief Put new data to the beginning of the buffer.
			 *
			 * @param buf
			 * @param len
			 */
			void AddInFront(const char *buf, size_t len);

			/**
			 * @brief Put a byte value to the beginning of the buffer.
			 *
			 * @param b
			 */
			void AddInFrontInt8(const uint8_t b)
			{
				AddInFront(static_cast<const char *>((void *)&b), 1);
			}

			/**
			 * @brief Put a unsigned short value to the beginning of the buffer.
			 *
			 * @param s
			 */
			void AddInFrontInt16(const uint16_t s);

			/**
			 * @brief Put a unsigned int value to the beginning of the buffer.
			 *
			 * @param i
			 */
			void AddInFrontInt32(const uint32_t i);

			/**
			 * @brief Put a unsigned int64 value to the beginning of the buffer.
			 *
			 * @param l
			 */
			void AddInFrontInt64(const uint64_t l);

			/**
			 * @brief Remove all data in the buffer.
			 *
			 */
			void RetrieveAll();

			/**
			 * @brief Remove some bytes in the buffer.
			 *
			 * @param len
			 */
			void Retrieve(size_t len);

			/**
			 * @brief Read data from a file descriptor and put it into the buffer.˝
			 *
			 * @param fd The file descriptor. It is usually a socket.
			 * @param retErrno The error code when reading.
			 * @return ssize_t The number of bytes read from the file descriptor. -1 is
			 * returned when an error occurs.
			 */
			//int32_t ReadFd(int fd, int *retErrno);

			/**
			 * @brief Remove the data before a certain position from the buffer.
			 *
			 * @param end The position.
			 */
			void RetrieveUntil(const char *end)
			{
				assert(Peek() <= end);
				assert(end <= BeginWrite());
				Retrieve(end - Peek());
			}

			/**
			 * @brief Find the position of the buffer where the CRLF is found.
			 *
			 * @return const char*
			 */
			const char *FindCRLF() const;
			//{
			//	const char *crlf = std::search(Peek(), BeginWrite(), CRLF, CRLF + 2);
			//	return crlf == BeginWrite() ? NULL : crlf;
			//}

			/**
			 * @brief Make sure the buffer has enough spaces to write data.
			 *
			 * @param len
			 */
			void EnsureWritableBytes(size_t len);

			/**
			 * @brief Move the write pointer forward when the new data has been written
			 * to the buffer.
			 *
			 * @param len
			 */
			void HasWritten(size_t len)
			{
				assert(len <= WritableBytes());
				tail_ += len;
			}

			/**
			 * @brief Move the write pointer backward to remove data in the end of the
			 * buffer.
			 *
			 * @param offset
			 */
			void Unwrite(size_t offset)
			{
				assert(ReadableBytes() >= offset);
				tail_ -= offset;
			}

			/**
			 * @brief Access a byte in the buffer.
			 *
			 * @param offset
			 * @return const char&
			 */
			const char &operator[](size_t offset) const
			{
				assert(ReadableBytes() >= offset);
				return Peek()[offset];
			}
			char &operator[](size_t offset)
			{
				assert(ReadableBytes() >= offset);
				return begin()[head_ + offset];
			}

		private:
			size_t head_;
			size_t initCap_;
			std::vector<char> buffer_;
			size_t tail_;
			const char *begin() const
			{
				return &buffer_[0];
			}
			char *begin()
			{
				return &buffer_[0];
			}
		};

		inline void swap(MsgBuffer &one, MsgBuffer &two) noexcept
		{
			one.Swap(two);
		}
		inline uint64_t hton64(uint64_t n)
		{
			static const int one = 1;
			static const char sig = *(char *)&one;
			if (sig == 0)
				return n;  // for big endian machine just return the input
			char *ptr = reinterpret_cast<char *>(&n);
			std::reverse(ptr, ptr + sizeof(uint64_t));
			return n;
		}
		inline uint64_t ntoh64(uint64_t n)
		{
			return hton64(n);
		}
	}
}

//#define _C_LIBHTTP_MSG_BUFFER_H_
#endif // 
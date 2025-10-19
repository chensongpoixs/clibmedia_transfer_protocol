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

#ifndef _C_LIBHTTP_TYPE_H_
#define _C_LIBHTTP_TYPE_H_

#include <algorithm>

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		enum HttpStatusCode
		{
			kUnknown = 0,
			k100Continue = 100,
			k101SwitchingProtocols = 101,
			k200OK = 200,
			k201Created = 201,
			k202Accepted = 202,
			k203NonAuthoritativeInformation = 203,
			k204NoContent = 204,
			k205ResetContent = 205,
			k206PartialContent = 206,
			k300MultipleChoices = 300,
			k301MovedPermanently = 301,
			k302Found = 302,
			k303SeeOther = 303,
			k304NotModified = 304,
			k305UseProxy = 305,
			k307TemporaryRedirect = 307,
			k308PermanentRedirect = 308,
			k400BadRequest = 400,
			k401Unauthorized = 401,
			k402PaymentRequired = 402,
			k403Forbidden = 403,
			k404NotFound = 404,
			k405MethodNotAllowed = 405,
			k406NotAcceptable = 406,
			k407ProxyAuthenticationRequired = 407,
			k408RequestTimeout = 408,
			k409Conflict = 409,
			k410Gone = 410,
			k411LengthRequired = 411,
			k412PreconditionFailed = 412,
			k413RequestEntityTooLarge = 413,
			k414RequestURITooLarge = 414,
			k415UnsupportedMediaType = 415,
			k416RequestedRangeNotSatisfiable = 416,
			k417ExpectationFailed = 417,
			k418ImATeapot = 418,
			k421MisdirectedRequest = 421,
			k425TooEarly = 425,
			k426UpgradeRequired = 426,
			k428PreconditionRequired = 428,
			k429TooManyRequests = 429,
			k431RequestHeaderFieldsTooLarge = 431,
			k451UnavailableForLegalReasons = 451,
			k500InternalServerError = 500,
			k501NotImplemented = 501,
			k502BadGateway = 502,
			k503ServiceUnavailable = 503,
			k504GatewayTimeout = 504,
			k505HTTPVersionNotSupported = 505,
			k510NotExtended = 510,
		};

		enum class Version
		{
			kUnknown = 0,
			kHttp10,
			kHttp11
		};

		enum ContentType
		{
			kContentTypeNONE = 0,
			kContentTypeAppJson,
			kContentTypeTextPlain,
			kContentTypeTextHTML,
			kContentTypeAppXForm,
			kContentTypeAppMpegUrl,
			kContentTypeVideoMP2T,
			kContentTypeVideoXFlv,
			kContentTypeTextXML,
			kContentTypeAppXML,
		};

		enum HttpMethod
		{
			kGet = 0,
			kPost,
			kHead,
			kPut,
			kDelete,
			kOptions,
			kPatch,
			kInvalid
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_
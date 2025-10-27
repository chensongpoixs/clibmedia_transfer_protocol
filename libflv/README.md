



# HTTP-FLV协议


@[TOC](HTTP-FLV协议)


</font>

![](img/flv_tag.png)

  

# 前言
 
http-flv 其实网络中发送flv文件， 也是网络格式





## 一、FLV简介
Flash Video（简称FLV），是一种网络视频格式，也是一种流媒体格式。

FLV文件由 FLV File Header + FLV File Body组成。

![](img/flv_tag.png)
 

## 二、FLV File Header

 FLV File Header固定为9个字节，用来描述版本号，有没有音视频
结构如下：

|名称	|比特数|	描述|
|:---:|:---:|:---:|
|Signature	|24	|签名，固定为“FLV”|
|Version	|8	|版本号，固定为0x01，表示FLV Version 1|
|TypeFlagsReserved	|5	|全0|
|TypeFlagsAudio	|1	|1表示有audio tag，0表示没有audio tag|
|TypeFlagsReserved	|1	|全0|
|TypeFlagsVideo	|1	|1表示有video tag，0表示没有video tag|
|DataOffset|	32	|FLV header的大小，单位是字节|


```javascript
struct FLVHeader{
	//FLV
	char flv[3];
	//File version (for example, 0x01 for FLV version 1)
	uint8_t version; 
	// 保留,置0   
	uint8_t : 5;
	// 是否有音频   
	uint8_t have_audio : 1;
	// 保留,置0   
	uint8_t : 1;
	// 是否有视频   
	uint8_t have_video : 1;
	固定为9   
	uint32_t length;
	// 固定为0   
	uint32_t previous_tag_size0;
};

```

## 三、FLV File Body

FLV File Body由 PreviousTagSize0+tag1+PreviousTagSize1+…+tagN+PreviousTagSizeN组成。
PreviousTagSize 是一个4字节的整数，表示前一个TAG的大小。

|PreviousTagSize0|tag1|PreviousTagSize1|PreviousTagSize1|tag2|PreviousTagSize2|...|tagN|PreviousTagSizeN|


```javascript
//tag header
		std::string tag_header;
		tag_header.append((char *)&header, sizeof(header));
		Writer((const uint8_t *)tag_header.c_str(), tag_header.size()); ///

		//tag data
		Writer(data, size);

		//PreviousTagSize
		uint32_t PreviousTag_Size = htonl((uint32_t)(size + sizeof(header)));
		std::string PreviousTagSize;
		PreviousTagSize.append((char *)&PreviousTag_Size, 4);
		Writer((const uint8_t *)PreviousTagSize.c_str(), PreviousTagSize.size(), true);

```

## 四、Flv Tag 存储音频和视频数据的地方

FLV tag由 tag header + tag body组成。

|Tag Header|Tag Body|


tag header固定为11个字节，结构如下：

|名称|	比特数|	描述|
|---:|:---:|:---:|
|TagType|	8|	tag类型 <br> 8：audio<br>9：video<br>18：script data<br>其他：保留|
|DataSize	|24|	message 长度，从StreamID后面到tag结束|
|Timestamp	|24	| <font color='red'>相对于第一个tag的时间戳（单位是 毫秒），第一个tag的Timestamp总为0</font>|
TimestampExtended	|8	|时间戳的扩展字段，当 Timestamp 3个字节不够时，会启用这个字段，代表高8位|
|StreamID	|24	|总是0|

```javascript

		struct FlvTagHeader {
		 
			uint8_t type = 0;
			uint8_t data_size[3] = { 0 };
			uint8_t timestamp[3] = { 0 };
			uint8_t timestamp_ex = 0;
			uint8_t streamid[3] = { 0 }; /* Always 0. */
		};
```

## 五、FLV Tag Body  

根据 tag type的值，tag body可以分为AUDIODATA（tag type为8），VIDEODATA（tag type为9），SCRIPTDATAOBJECT（tag type为18）

### 1、AUDIODATA

AUDIODATA 承载音频数据，第一个字节描述音频的信息，第二个字节开始为音频数据。

#### ① AUDIODATA的结构如下：


|名称	|比特数|	描述|
|:---:|:---:|:---:|
|SoundFormat	|4	|音频编码格式|
|SoundRate	|2	|采样率|
|SoundSize	|1|	采样精度，0表示8-bit，1表示16-bit|
|SoundType	|1	|声道类型，0表示单声道，1表示立体声|
|SoundData|	N*8	|音频数据|


#### ② 音频编码格式：

|ID值|音频编码|
| :---:| :---:|
|0|Linear PCM, platform endian|
|1|ADPCM|
|2|MP3|
|3|Linear PCM, little endian|
|4|Nellymoser 16 kHz mono|
|5|Nellymoser 8 kHz mono|
|6|Nellymoser|
|7|G.711 A-law logarithmic PCM|
|8|G.711 mu-law logarithmic PCM|
|9|reserved|
|10|AAC|
|11|Speex|
|13|Opus|
|14|MP3 8 kHz|
|15|Device-specific sound|

####  ③ 当SoundFormat=10，即AAC音频时，SoundData的第一个字节为AACPacketType。AACPacketType为0表示这个音频包是AudioSpecificConfig；否则这个音频包为AAC帧数据。
AudioSpecificConfig的结构：

|名称	|比特数|	描述|
|:---:|:---:|:---:|
|AudioObjectType|	5	|音频对象类型|
|SamplingFrequencyIndex	|4|	采样率索引值，比如4表示44100|
|ChannelConfiguration|	4	|声道配置|

AudioObjectType的取值：

|值|	ObjectType|
|:---:|:---:|
|1	|AAC Main|
|2	|AAC LC|
|3	|AAC SSR|
|5	|AAC HE|
|29	|AAC HEV2|


SamplingFrequency的取值：

|sampling frequency index	|frequency|
|:---:|:---|
|0x0	|96000|
|0x1	|88200|
|0x2|	64000|
|0x3	|48000|
|0x4	|44100|
|0x5	|32000|
|0x6|	24000|
|0x7	|22050|
|0x8	|16000|
|0x9	|12000|
|0xa	|11025|
|0xb	|8000|
|0xc	|7350|
|0xd	|reserved|
|0xe	|reserved|
|0xf	|escape value|

SamplingFrequencyIndex是SamplingFrequency数组的一个索引。

当SoundFormat=2，即MP3音频时，SoundData就是MP3 RAW数据

### 2、VideoData

VIDEODATA Tag第一个字节的高4位描述视频帧的类型，低4位描述视频编码器ID，VIDEODATA Tag的结构如下：

|名称	|比特数	|描述|
|:---:|:---:|:---:|
|FrameType	|4|	帧类型|
|CodecID|	4	|视频编码ID|
|VideoData	|N*8	|视频数据|

```javascript
uint8_t *ptr = buffer;
	*ptr = FLV_CODECID_H264;
	*ptr++ |= FLV_FRAME_KEY;

```



FrameType：视频帧的类型。一般keyframe是指IDR帧，而inter frame是指普通I帧。

|类型值|	视频帧|
|:---:|:---:|
|1	|key frame (for AVC, a seekable frame)|
|2	|inter frame (for AVC, a non-seekable frame)|
|3	|disposable inter frame (H.263 only)|
|4	|generated key frame (reserved for server use only)|
|5	|video info/command frame|



视频编码ID：

|ID值	|视频编码|
|:---:|:---:|
|2	|Sorenson H.263|
|3|	Screen video|
|4|	On2 VP6|
|5	|On2 VP6 with alpha channel|
|6	|Screen video version 2|
|7	|AVC|


<font color='red'>当CodecID 为7时，即为AVC视频，第一个字节为AvcPacketType，第二三四个字节为CompositionTime。当AvcPacketType=0，第5个字节开始为AVCDecoderConfigurationRecord；否则VideoData为Avc Raw数据。
AVCDecoderConfigurationRecord的结构：

|名称|	比特数	|描述|
|:---:|:---:|:---:|
|configurationVersion	|8	|版本号，总是1|
|AVCProfileIndication|	8	|sps[1]|
|profile_compatibility	|8	|sps[2]|
|AVCLevelIndication|	8	|sps[3]|

- configurationVersion,AVCProfileIndication,profile_compatibility,AVCLevelIndication：都是一个字节，具体的内容由解码器去理解。
- lengthSizeMinusOne：unit_length长度所占的字节数减1，也即lengthSizeMinusOne的值+1才是unit_length所占用的字节数。
- numOfSequenceParameterSets：sps的个数
- sequenceParameterSetLength：sps内容的长度
- sequenceParameterSetNALUnit：sps的内容
- numOfPictureParameterSets：pps的个数
- pictureParameterSetLength：pps内容的长度
- pictureParameterSetNALUnit：pps的内容

```javascript

std::string extra_data;
	{

		/*
		configurationVersion	8	版本号，总是1
		AVCProfileIndication	8	sps[1]
		profile_compatibility	8	sps[2]
		AVCLevelIndication	8	sps[3]
		configurationVersion,AVCProfileIndication,profile_compatibility,AVCLevelIndication：都是一个字节，具体的内容由解码器去理解。
		lengthSizeMinusOne：unit_length长度所占的字节数减1，也即lengthSizeMinusOne的值+1才是unit_length所占用的字节数。
		numOfSequenceParameterSets：sps的个数
		sequenceParameterSetLength：sps内容的长度
		sequenceParameterSetNALUnit：sps的内容
		numOfPictureParameterSets：pps的个数
		pictureParameterSetLength：pps内容的长度
		pictureParameterSetNALUnit：pps的内容
		*/
		// AVCDecoderConfigurationRecord start
		extra_data.push_back(1); // version
	//	*ptr++ = 1;
		extra_data.push_back(sps_[1]); // profile
		//*ptr++ = sps_[1];
		extra_data.push_back(sps_[2]); // compat
		//*ptr++ = sps_[2];
		extra_data.push_back(sps_[3]); // level
		//*ptr++ = sps_[3];
		extra_data.push_back((char)0xff); // 6 bits reserved + 2 bits nal size length - 1 (11)
		//*ptr++ = 0xff;
		extra_data.push_back((char)0xe1); // 3 bits reserved + 5 bits number of sps (00001)
		//*ptr++ = 0xe1;
		// sps
		uint16_t size = (uint16_t)sps_.size();
		size = htons(size);
		extra_data.append((char *)&size, 2);
		extra_data.append(sps_);
		
		// pps
		extra_data.push_back(1); // version
		size = (uint16_t)pps_.size();
		size = htons(size);
		extra_data.append((char *)&size, 2);
		extra_data.append(pps_);

	}

	// memcpy()
	//packet.append(extra_data);
	memcpy(ptr, extra_data.c_str(), extra_data.size());
	ptr += extra_data.size();
	WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer, ptr - buffer, 0);
```

当VideoData为AVC RAW时，AVC RAW的结构是avcc的

#### 3、  Script OnMeta （实际解码拿sps和pps中信息解析解码的， 所以该字段没有啥作用)

Script Data Tags通常用来存放跟FLV中音视频相关的元数据信息（onMetaData），比如时长、长度、宽度等。它的定义相对复杂些，采用AMF（Action Message Format）封装了一系列数据类型，比如字符串、数值、数组等。

onMetaData中包含了音视频相关的元数据，封装在Script Data Tag中，它包含了两个AMF。

第一个AMF是固定的值：

```
0x02 0x000A 0x6F 0x6E 0x4D 0x65 0x74 0x61 0x44 0x61 0x74 0x61
```

- 第1个字节：0x02，表示字符串类型
- 第2-3个字节：UI16类型，值为0x000A，表示字符串的长度为10（onMetaData的长度）；
- 第4-13个字节：字符串onMetaData对应的16进制数字（0x6F 0x6E 0x4D 0x65 0x74 0x61 0x44 0x61 0x74 0x61）；

第二个AMF则是键值对描述的流媒体属性，每个实现这些属性都可能不一样：

|字段|	字段类型	|字段含义|
|:---:|:---|:---:|
|duration	|DOUBLE|	文件的时长|
|width|	DOUBLE|	视频宽度（px）|
|height	|DOUBLE|	视频高度（px）|
|videodatarate|	DOUBLE	|视频比特率（kb/s）|
|framerate	|DOUBLE	|视频帧率（帧/s）|
|videocodecid	|DOUBLE|	视频编解码器ID（参考Video Tag）|
|audiosamplerate	|DOUBLE|	音频采样率|
|audiosamplesize|	DOUBLE|	音频采样精度（参考Audio Tag）|
|stereo	|BOOL|	是否立体声|
|audiocodecid	|DOUBLE	|音频编解码器ID（参考Audio Tag）|
|filesize	|DOUBLE	|文件总得大小（字节）|

```javascript 

//prev_packet_size_ = index;
			uint8_t * metadata = current_ ;
			uint8_t * ptr = metadata;
			uint8_t *end = ptr + (1024 * 1023);

			uint8_t count = (has_auido ? 5 : 0) + (has_video ? 7 : 0) + 1;
			ptr = AMFWriteString(ptr, end, "onMetaData", 10);
			// value: SCRIPTDATAECMAARRAY
			ptr[0] = AMF_ECMA_ARRAY;
			ptr[1] = (uint8_t)((count >> 24) & 0xFF);;
			ptr[2] = (uint8_t)((count >> 16) & 0xFF);;
			ptr[3] = (uint8_t)((count >> 8) & 0xFF);
			ptr[4] = (uint8_t)(count & 0xFF);
			ptr += 5;


			if (has_auido)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "audiocodecid", 12, 10);
				ptr = AMFWriteNamedDouble(ptr, end, "audiodatarate", 13, 125 /* / 1024.0*/);
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplerate", 15, 44100);
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplesize", 15, 16);
				ptr = AMFWriteNamedBoolean(ptr, end, "stereo", 6, (uint8_t)true);
			}
		
			if (has_video)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "duration", 8, 0 );
				//ptr = AMFWriteNamedDouble(ptr, end, "interval", 8, metadata->interval);
				ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, 7);
				ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, 0 /* / 1024.0*/);
				ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, 25);
				ptr = AMFWriteNamedDouble(ptr, end, "height", 6, 2560);
				ptr = AMFWriteNamedDouble(ptr, end, "width", 5, 1440);
			}
			ptr = AMFWriteNamedString(ptr, end, "encoder", 7, kflv_muxer, strlen(kflv_muxer));
			ptr = AMFWriteObjectEnd(ptr, end);
			 
			WriteFlvTag(libflv::kFlvMsgTypeAMFMeta, metadata, ptr - metadata, 0);
```


解码效果图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/869bf79ce5574bdaaad5829a55754298.png)


# 总结 

[libflv 源码地址：https://github.com/chensongpoixs/libmedia_transfer_protocol/tree/master/libflv](https://github.com/chensongpoixs/libmedia_transfer_protocol/tree/master/libflv)
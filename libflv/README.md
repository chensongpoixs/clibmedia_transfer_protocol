



# HTTP-FLVЭ��


@[TOC](HTTP-FLVЭ��)


</font>

![](img/flv_tag.png)

  

# ǰ��
 
http-flv ��ʵ�����з���flv�ļ��� Ҳ�������ʽ





## һ��FLV���
Flash Video�����FLV������һ��������Ƶ��ʽ��Ҳ��һ����ý���ʽ��

FLV�ļ��� FLV File Header + FLV File Body��ɡ�

![](img/flv_tag.png)
 

## ����FLV File Header

 FLV File Header�̶�Ϊ9���ֽڣ����������汾�ţ���û������Ƶ
�ṹ���£�

|����	|������|	����|
|:---:|:---:|:---:|
|Signature	|24	|ǩ�����̶�Ϊ��FLV��|
|Version	|8	|�汾�ţ��̶�Ϊ0x01����ʾFLV Version 1|
|TypeFlagsReserved	|5	|ȫ0|
|TypeFlagsAudio	|1	|1��ʾ��audio tag��0��ʾû��audio tag|
|TypeFlagsReserved	|1	|ȫ0|
|TypeFlagsVideo	|1	|1��ʾ��video tag��0��ʾû��video tag|
|DataOffset|	32	|FLV header�Ĵ�С����λ���ֽ�|


```javascript
struct FLVHeader{
	//FLV
	char flv[3];
	//File version (for example, 0x01 for FLV version 1)
	uint8_t version; 
	// ����,��0   
	uint8_t : 5;
	// �Ƿ�����Ƶ   
	uint8_t have_audio : 1;
	// ����,��0   
	uint8_t : 1;
	// �Ƿ�����Ƶ   
	uint8_t have_video : 1;
	�̶�Ϊ9   
	uint32_t length;
	// �̶�Ϊ0   
	uint32_t previous_tag_size0;
};

```

## ����FLV File Body

FLV File Body�� PreviousTagSize0+tag1+PreviousTagSize1+��+tagN+PreviousTagSizeN��ɡ�
PreviousTagSize ��һ��4�ֽڵ���������ʾǰһ��TAG�Ĵ�С��

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

## �ġ�Flv Tag �洢��Ƶ����Ƶ���ݵĵط�

FLV tag�� tag header + tag body��ɡ�

|Tag Header|Tag Body|


tag header�̶�Ϊ11���ֽڣ��ṹ���£�

|����|	������|	����|
|---:|:---:|:---:|
|TagType|	8|	tag���� <br> 8��audio<br>9��video<br>18��script data<br>����������|
|DataSize	|24|	message ���ȣ���StreamID���浽tag����|
|Timestamp	|24	| <font color='red'>����ڵ�һ��tag��ʱ�������λ�� ���룩����һ��tag��Timestamp��Ϊ0</font>|
TimestampExtended	|8	|ʱ�������չ�ֶΣ��� Timestamp 3���ֽڲ���ʱ������������ֶΣ������8λ|
|StreamID	|24	|����0|

```javascript

		struct FlvTagHeader {
		 
			uint8_t type = 0;
			uint8_t data_size[3] = { 0 };
			uint8_t timestamp[3] = { 0 };
			uint8_t timestamp_ex = 0;
			uint8_t streamid[3] = { 0 }; /* Always 0. */
		};
```

## �塢FLV Tag Body  

���� tag type��ֵ��tag body���Է�ΪAUDIODATA��tag typeΪ8����VIDEODATA��tag typeΪ9����SCRIPTDATAOBJECT��tag typeΪ18��

### 1��AUDIODATA

AUDIODATA ������Ƶ���ݣ���һ���ֽ�������Ƶ����Ϣ���ڶ����ֽڿ�ʼΪ��Ƶ���ݡ�

#### �� AUDIODATA�Ľṹ���£�


|����	|������|	����|
|:---:|:---:|:---:|
|SoundFormat	|4	|��Ƶ�����ʽ|
|SoundRate	|2	|������|
|SoundSize	|1|	�������ȣ�0��ʾ8-bit��1��ʾ16-bit|
|SoundType	|1	|�������ͣ�0��ʾ��������1��ʾ������|
|SoundData|	N*8	|��Ƶ����|


#### �� ��Ƶ�����ʽ��

|IDֵ|��Ƶ����|
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

####  �� ��SoundFormat=10����AAC��Ƶʱ��SoundData�ĵ�һ���ֽ�ΪAACPacketType��AACPacketTypeΪ0��ʾ�����Ƶ����AudioSpecificConfig�����������Ƶ��ΪAAC֡���ݡ�
AudioSpecificConfig�Ľṹ��

|����	|������|	����|
|:---:|:---:|:---:|
|AudioObjectType|	5	|��Ƶ��������|
|SamplingFrequencyIndex	|4|	����������ֵ������4��ʾ44100|
|ChannelConfiguration|	4	|��������|

AudioObjectType��ȡֵ��

|ֵ|	ObjectType|
|:---:|:---:|
|1	|AAC Main|
|2	|AAC LC|
|3	|AAC SSR|
|5	|AAC HE|
|29	|AAC HEV2|


SamplingFrequency��ȡֵ��

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

SamplingFrequencyIndex��SamplingFrequency�����һ��������

��SoundFormat=2����MP3��Ƶʱ��SoundData����MP3 RAW����

### 2��VideoData

VIDEODATA Tag��һ���ֽڵĸ�4λ������Ƶ֡�����ͣ���4λ������Ƶ������ID��VIDEODATA Tag�Ľṹ���£�

|����	|������	|����|
|:---:|:---:|:---:|
|FrameType	|4|	֡����|
|CodecID|	4	|��Ƶ����ID|
|VideoData	|N*8	|��Ƶ����|

```javascript
uint8_t *ptr = buffer;
	*ptr = FLV_CODECID_H264;
	*ptr++ |= FLV_FRAME_KEY;

```



FrameType����Ƶ֡�����͡�һ��keyframe��ָIDR֡����inter frame��ָ��ͨI֡��

|����ֵ|	��Ƶ֡|
|:---:|:---:|
|1	|key frame (for AVC, a seekable frame)|
|2	|inter frame (for AVC, a non-seekable frame)|
|3	|disposable inter frame (H.263 only)|
|4	|generated key frame (reserved for server use only)|
|5	|video info/command frame|



��Ƶ����ID��

|IDֵ	|��Ƶ����|
|:---:|:---:|
|2	|Sorenson H.263|
|3|	Screen video|
|4|	On2 VP6|
|5	|On2 VP6 with alpha channel|
|6	|Screen video version 2|
|7	|AVC|


<font color='red'>��CodecID Ϊ7ʱ����ΪAVC��Ƶ����һ���ֽ�ΪAvcPacketType���ڶ����ĸ��ֽ�ΪCompositionTime����AvcPacketType=0����5���ֽڿ�ʼΪAVCDecoderConfigurationRecord������VideoDataΪAvc Raw���ݡ�
AVCDecoderConfigurationRecord�Ľṹ��

|����|	������	|����|
|:---:|:---:|:---:|
|configurationVersion	|8	|�汾�ţ�����1|
|AVCProfileIndication|	8	|sps[1]|
|profile_compatibility	|8	|sps[2]|
|AVCLevelIndication|	8	|sps[3]|

- configurationVersion,AVCProfileIndication,profile_compatibility,AVCLevelIndication������һ���ֽڣ�����������ɽ�����ȥ��⡣
- lengthSizeMinusOne��unit_length������ռ���ֽ�����1��Ҳ��lengthSizeMinusOne��ֵ+1����unit_length��ռ�õ��ֽ�����
- numOfSequenceParameterSets��sps�ĸ���
- sequenceParameterSetLength��sps���ݵĳ���
- sequenceParameterSetNALUnit��sps������
- numOfPictureParameterSets��pps�ĸ���
- pictureParameterSetLength��pps���ݵĳ���
- pictureParameterSetNALUnit��pps������

```javascript

std::string extra_data;
	{

		/*
		configurationVersion	8	�汾�ţ�����1
		AVCProfileIndication	8	sps[1]
		profile_compatibility	8	sps[2]
		AVCLevelIndication	8	sps[3]
		configurationVersion,AVCProfileIndication,profile_compatibility,AVCLevelIndication������һ���ֽڣ�����������ɽ�����ȥ��⡣
		lengthSizeMinusOne��unit_length������ռ���ֽ�����1��Ҳ��lengthSizeMinusOne��ֵ+1����unit_length��ռ�õ��ֽ�����
		numOfSequenceParameterSets��sps�ĸ���
		sequenceParameterSetLength��sps���ݵĳ���
		sequenceParameterSetNALUnit��sps������
		numOfPictureParameterSets��pps�ĸ���
		pictureParameterSetLength��pps���ݵĳ���
		pictureParameterSetNALUnit��pps������
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

��VideoDataΪAVC RAWʱ��AVC RAW�Ľṹ��avcc��

#### 3��  Script OnMeta ��ʵ�ʽ�����sps��pps����Ϣ��������ģ� ���Ը��ֶ�û��ɶ����)

Script Data Tagsͨ��������Ÿ�FLV������Ƶ��ص�Ԫ������Ϣ��onMetaData��������ʱ�������ȡ���ȵȡ����Ķ�����Ը���Щ������AMF��Action Message Format����װ��һϵ���������ͣ������ַ�������ֵ������ȡ�

onMetaData�а���������Ƶ��ص�Ԫ���ݣ���װ��Script Data Tag�У�������������AMF��

��һ��AMF�ǹ̶���ֵ��

```
0x02 0x000A 0x6F 0x6E 0x4D 0x65 0x74 0x61 0x44 0x61 0x74 0x61
```

- ��1���ֽڣ�0x02����ʾ�ַ�������
- ��2-3���ֽڣ�UI16���ͣ�ֵΪ0x000A����ʾ�ַ����ĳ���Ϊ10��onMetaData�ĳ��ȣ���
- ��4-13���ֽڣ��ַ���onMetaData��Ӧ��16�������֣�0x6F 0x6E 0x4D 0x65 0x74 0x61 0x44 0x61 0x74 0x61����

�ڶ���AMF���Ǽ�ֵ����������ý�����ԣ�ÿ��ʵ����Щ���Զ����ܲ�һ����

|�ֶ�|	�ֶ�����	|�ֶκ���|
|:---:|:---|:---:|
|duration	|DOUBLE|	�ļ���ʱ��|
|width|	DOUBLE|	��Ƶ��ȣ�px��|
|height	|DOUBLE|	��Ƶ�߶ȣ�px��|
|videodatarate|	DOUBLE	|��Ƶ�����ʣ�kb/s��|
|framerate	|DOUBLE	|��Ƶ֡�ʣ�֡/s��|
|videocodecid	|DOUBLE|	��Ƶ�������ID���ο�Video Tag��|
|audiosamplerate	|DOUBLE|	��Ƶ������|
|audiosamplesize|	DOUBLE|	��Ƶ�������ȣ��ο�Audio Tag��|
|stereo	|BOOL|	�Ƿ�������|
|audiocodecid	|DOUBLE	|��Ƶ�������ID���ο�Audio Tag��|
|filesize	|DOUBLE	|�ļ��ܵô�С���ֽڣ�|

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


����Ч��ͼ
![���������ͼƬ����](https://i-blog.csdnimg.cn/direct/869bf79ce5574bdaaad5829a55754298.png)


# �ܽ� 

[libflv Դ���ַ��https://github.com/chensongpoixs/libmedia_transfer_protocol/tree/master/libflv](https://github.com/chensongpoixs/libmedia_transfer_protocol/tree/master/libflv)


# FLV





![](img/flv_tag.png)

1. FLVֻ��һ�ַ�װ��ʽ
2. HTTP-FLV��ͨ��HTTP�����FLV�ļ�
3. RTMP�����FLVʵʱ���ݣ� ȱ�ٵ�һ��TAG


# ���� FLV�ļ�ͷ


1. 9���ֽڵ�TAG�������汾�ţ� Я����������û����Ƶ�� ��û����Ƶ
2. ����Ƶ��������ϣ� ֻ����Ƶ��ֻ����Ƶ��������Ƶ����
3. FLV�ļ�ͷ������Ϊ��һ�������ȷ���



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


 

# ����FLV��TAG

1. TAG��ʼǰ����һ��4�ֽڵĳ��ȣ� ��ʾ��һ��TAG�ĳ���
2. TAG��һ���ֽڵĵ�5λΪ�����ͣ� ��Ƶ8�� ��Ƶ9�� Meta18. ����λ��0������λ����Ӳ����ܣ�
3. TAG��2-4�ֽ�λTAG���ݳ��ȣ� Ҳ�����ݰ��ĳ���
4. ��������3���ֽڵ�TimeStamp +1 ���ֽڵ�TimeStampExtended
5. �����3���ֽڵ�StreamID��������Ϊ0


```
struct FlvTagHeader {
		 
	uint8_t type = 0;
	uint8_t data_size[3] = { 0 };
	uint8_t timestamp[3] = { 0 };
	uint8_t timestamp_ex = 0;
	uint8_t streamid[3] = { 0 }; /* Always 0. */
};

```





#  �ġ�onMeta



#  �塢 configDecoder �������Ҫ��ȡ����������Ϣ sps��pps ����Ƶ������


H264���ݸ�ʽ����AVCC��ʽ



## 1�� AVCDecoderConfigurationRecord��Ƶ������Ϣsps��ppsconfig


```
uint8_t *ptr = buffer;
*ptr = FLV_CODECID_H264;
*ptr++ |= FLV_FRAME_KEY;
//config ������Ϣ����Ϊ0 
*ptr++ = 0;
*ptr++ = 0;
*ptr++ = 0;
*ptr++ = 0;
// cts 
// AVCDecoderConfigurationRecord start
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
```

## 2�� ������nal�����ݵķ��͸�ʽ

```

 // ������tag hreader
FlvTagHeader  
{
  ����9
  ʱ���ts;
  ��Ƶ��id  Ĭ��0

  }


 

 tag body��������
 enum {
    FLV_FRAME_KEY            = 1 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< key frame (for AVC, a seekable frame)
    FLV_FRAME_INTER          = 2 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< inter frame (for AVC, a non-seekable frame)
    FLV_FRAME_DISP_INTER     = 3 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< disposable inter frame (H.263 only)
    FLV_FRAME_GENERATED_KEY  = 4 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< generated key frame (reserved for server use only)
    FLV_FRAME_VIDEO_INFO_CMD = 5 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< video info/command frame
};

//��Ƶ֡������H264 �ֱ�Ϊ���� �ؼ���ʹ��FLV_FRAME_KEY  �ǹؼ���ʹ��FLV_FRAME_INTER
avio_w8(pb, par->codec_tag | FLV_FRAME_KEY); // flags
avio_w8(pb, 0); // AVC sequence header
avio_wb24(pb, 0); // composition time
// avcc ��ʽ |4byte����|��������|
д��avcc���� avio_write(data, size);  

data_size = avio_tell(pb) - pos;
avio_seek(pb, -data_size - 10, SEEK_CUR);
avio_wb24(pb, data_size);
avio_skip(pb, data_size + 10 - 3);
//����tag header + tag body �Ĵ�С  =>   11 FlvTagHeader==> 11 
avio_wb32(pb, data_size + 11); // previous tag size




```


## 1��







FlvHeader


```
struct FlvTagHeader {
		 
	uint8_t type = 0;
	uint8_t data_size[3] = { 0 };
	uint8_t timestamp[3] = { 0 };
	uint8_t timestamp_ex = 0;
	uint8_t streamid[3] = { 0 }; /* Always 0. */
};

```


TagHeader


```

```



TagData

```


```







#  HTTP��Ϣͷ


```

HTTP/1.1 200 OK \r\n
Access-Control-Allow-Origin:*\r\n
Content-Type: video/x-flv\r\n
Connection:Keep-Alive\r\n
\r\n

```
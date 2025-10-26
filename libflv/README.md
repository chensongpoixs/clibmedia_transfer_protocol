

# FLV





![](img/flv_tag.png)

1. FLV只是一种封装格式
2. HTTP-FLV是通过HTTP传输的FLV文件
3. RTMP传输的FLV实时数据， 缺少第一个TAG


# 二、 FLV文件头


1. 9个字节的TAG，描述版本号， 携带的数据有没有音频， 有没有视频
2. 音视频有三种组合： 只有音频、只有视频或者音视频都有
3. FLV文件头总是作为第一个数据先发送



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


 

# 三、FLV的TAG

1. TAG开始前总是一个4字节的长度， 表示上一个TAG的长度
2. TAG第一个字节的低5位为包类型： 音频8， 视频9， Meta18. 高三位置0（高三位代表加不加密）
3. TAG第2-4字节位TAG数据长度， 也即数据包的长度
4. 接下来是3个字节的TimeStamp +1 个字节的TimeStampExtended
5. 最后是3个字节的StreamID。总是置为0


```
struct FlvTagHeader {
		 
	uint8_t type = 0;
	uint8_t data_size[3] = { 0 };
	uint8_t timestamp[3] = { 0 };
	uint8_t timestamp_ex = 0;
	uint8_t streamid[3] = { 0 }; /* Always 0. */
};

```





#  四、onMeta



#  五、 configDecoder 解码端需要获取解码器的信息 sps、pps （视频举例）


H264数据格式按照AVCC格式



## 1、 AVCDecoderConfigurationRecord视频编码信息sps和ppsconfig


```
uint8_t *ptr = buffer;
*ptr = FLV_CODECID_H264;
*ptr++ |= FLV_FRAME_KEY;
//config 编码信息设置为0 
*ptr++ = 0;
*ptr++ = 0;
*ptr++ = 0;
*ptr++ = 0;
// cts 
// AVCDecoderConfigurationRecord start
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
```

## 2、 正常的nal的数据的发送格式

```

 // 正常的tag hreader
FlvTagHeader  
{
  类型9
  时间戳ts;
  视频流id  默认0

  }


 

 tag body数据区域
 enum {
    FLV_FRAME_KEY            = 1 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< key frame (for AVC, a seekable frame)
    FLV_FRAME_INTER          = 2 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< inter frame (for AVC, a non-seekable frame)
    FLV_FRAME_DISP_INTER     = 3 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< disposable inter frame (H.263 only)
    FLV_FRAME_GENERATED_KEY  = 4 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< generated key frame (reserved for server use only)
    FLV_FRAME_VIDEO_INFO_CMD = 5 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< video info/command frame
};

//视频帧的类型H264 分别为两种 关键字使用FLV_FRAME_KEY  非关键字使用FLV_FRAME_INTER
avio_w8(pb, par->codec_tag | FLV_FRAME_KEY); // flags
avio_w8(pb, 0); // AVC sequence header
avio_wb24(pb, 0); // composition time
// avcc 格式 |4byte长度|数据区域|
写入avcc数据 avio_write(data, size);  

data_size = avio_tell(pb) - pos;
avio_seek(pb, -data_size - 10, SEEK_CUR);
avio_wb24(pb, data_size);
avio_skip(pb, data_size + 10 - 3);
//整个tag header + tag body 的大小  =>   11 FlvTagHeader==> 11 
avio_wb32(pb, data_size + 11); // previous tag size




```


## 1、







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







#  HTTP消息头


```

HTTP/1.1 200 OK \r\n
Access-Control-Allow-Origin:*\r\n
Content-Type: video/x-flv\r\n
Connection:Keep-Alive\r\n
\r\n

```
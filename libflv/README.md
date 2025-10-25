

# FLV


1. FLV只是一种封装格式
2. HTTP-FLV是通过HTTP传输的FLV文件
3. RTMP传输的FLV实时数据， 缺少第一个TAG


# 二、 FLV文件头


1. 9个字节的TAG，描述版本号， 携带的数据有没有音频， 有没有视频
2. 音视频有三种组合： 只有音频、只有视频或者音视频都有
3. FLV文件头总是作为第一个数据先发送

# 三、 FLV只有音频的FLV头



```

static flv_audio_only_header[] = {
	0X46, /*'F'*/
	0X4C, /*'L*/
	0X56, /*'V'*/
	0X01, /* version = 1*/
	0X04, 
	0X00,
	0X00,
	0X00,
	0X09, /*header seize*/

};

```


视频flv头


```
static flv_video_only_header[] = {
	0X46, /*'F'*/
	0X4C, /*'L*/
	0X56, /*'V'*/
	0X01, /* version = 1*/
	0X01, 
	0X00,
	0X00,
	0X00,
	0X09, /*header seize*/

};
```


音视频flv头

```
static flv_header[] = {
	0X46, /*'F'*/
	0X4C, /*'L*/
	0X56, /*'V'*/
	0X01, /* version = 1*/
	0X05, /*  0000 0101 = has audio & video */
	0X00,
	0X00,
	0X00,
	0X09, /*header seize*/

};
`


# FLV的TAG

1. TAG开始前总是一个4字节的长度， 表示上一个TAG的长度
2. TAG第一个字节的低5位为包类型： 音频8， 视频9， Meta18. 高三位置0（高三位代表加不加密）
3. TAG第2-4字节位TAG数据长度， 也即数据包的长度
4. 接下来是3个字节的TimeStamp +1 个字节的TimeStampExtended
5. 最后是3个字节的StreamID。总是置为0


#  HTTP消息头


```

HTTP/1.1 200 OK \r\n
Access-Control-Allow-Origin:*\r\n
Content-Type: video/x-flv\r\n
Connection:Keep-Alive\r\n
\r\n

```





# FLV


1. FLVֻ��һ�ַ�װ��ʽ
2. HTTP-FLV��ͨ��HTTP�����FLV�ļ�
3. RTMP�����FLVʵʱ���ݣ� ȱ�ٵ�һ��TAG


# ���� FLV�ļ�ͷ


1. 9���ֽڵ�TAG�������汾�ţ� Я����������û����Ƶ�� ��û����Ƶ
2. ����Ƶ��������ϣ� ֻ����Ƶ��ֻ����Ƶ��������Ƶ����
3. FLV�ļ�ͷ������Ϊ��һ�������ȷ���

# ���� FLVֻ����Ƶ��FLVͷ



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


��Ƶflvͷ


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


����Ƶflvͷ

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


# FLV��TAG

1. TAG��ʼǰ����һ��4�ֽڵĳ��ȣ� ��ʾ��һ��TAG�ĳ���
2. TAG��һ���ֽڵĵ�5λΪ�����ͣ� ��Ƶ8�� ��Ƶ9�� Meta18. ����λ��0������λ����Ӳ����ܣ�
3. TAG��2-4�ֽ�λTAG���ݳ��ȣ� Ҳ�����ݰ��ĳ���
4. ��������3���ֽڵ�TimeStamp +1 ���ֽڵ�TimeStampExtended
5. �����3���ֽڵ�StreamID��������Ϊ0


#  HTTP��Ϣͷ


```

HTTP/1.1 200 OK \r\n
Access-Control-Allow-Origin:*\r\n
Content-Type: video/x-flv\r\n
Connection:Keep-Alive\r\n
\r\n

```



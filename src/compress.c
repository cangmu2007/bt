#include "compress.h"

int gzcompress(const int8_t* src,uint srclen,int8_t* des,uint deslen)
{
	 z_stream strm;
    //初始化strm结构中的zalloc, zfree, opaque,要求使用默认的内存分配策略
     strm.zalloc = Z_NULL;
     strm.zfree  = Z_NULL;
     strm.opaque = Z_NULL;

    //设置输入输出缓冲区
     strm.avail_in = srclen;
     strm.avail_out = deslen;
     strm.next_in = (Bytef *)src;
     strm.next_out = (Bytef *)des;

     int err = -1;
    //初始化zlib的状态，成功返回Z_OK
    //deflateInit:zlib格式，deflateInit2:gzip格式
	//err = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
     err = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS+16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
     if (err == Z_OK)
     {
			//Z_FINISH表明完成输入，让deflate()完成输出
			err = deflate(&strm, Z_FINISH);
			if(err == Z_STREAM_END)
			{
				//deflateEnd释放资源，防止内存泄漏
				deflateEnd(&strm);           
				//strm.avail_out表明输出缓冲区剩余的空闲空间大小
				return deslen - strm.avail_out;	
			}
			else
			{
				deflateEnd(&strm);
				return -1;
			}
     }
     else
     {
          deflateEnd(&strm);
          return -1;
     }
}

int decompress(const int8_t* src,uint srclen,int8_t* des,uint deslen)
{
	z_stream strm;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;

    strm.avail_in = srclen;
    strm.avail_out = deslen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)des;

    int err = -1;
	//err = inflateInit(&strm);
    err = inflateInit2(&strm, MAX_WBITS+16);
    if (err == Z_OK)
    {
         err = inflate(&strm, Z_FINISH);
         if (err == Z_STREAM_END)
         {
              inflateEnd(&strm);
              return strm.total_out;
         }
         else
         {
              inflateEnd(&strm);
              return -1;
         }
    }
    else
    {
         inflateEnd(&strm);
         return -1;
    }
}

int8_t* xml_compress(int8_t* src,int flag)
{
	uint srclen=strlen(src);

	//gzip头为10，尾是8
	uint deslen=compressBound(srclen)+10+8;

	int8_t* buf=(int8_t*)malloc(deslen+10);
	if(NULL==buf)
		return "FAULT";
	memset(buf,0,deslen+10);
	int reallen=-1;
	if((reallen=gzcompress(src,srclen,buf+10,deslen))<0)
	{
		free(buf);
		return "FAULT";
	}
	if(flag)
	{
		free(src);
		src=NULL;
	}
	
	//实际长度
	sprintf(buf,"%u",reallen);

	return buf;
}
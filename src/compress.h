#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <stdlib.h>  
#include <string.h>  
#include <stdio.h>  
#include <zlib.h>
#include <stdint.h>

//gz压缩，返回值为输出缓冲区剩余空间大小，小于0则为出错
int gzcompress(const int8_t* src,uint srclen,int8_t* des,uint deslen);

//gz解压，返回值为输出缓冲区剩余空间大小，小于0则为出错
int decompress(const int8_t* src,uint srclen,int8_t* des,uint deslen);

//xml字符串压缩,flag非0表示释放src空间
int8_t* xml_compress(int8_t* src,int flag);

#endif
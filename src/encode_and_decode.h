#ifndef __END_H__
#define __END_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/md5.h>

static const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//MD5编码（32）
int md5_encode(char* src,uint srclen,char* des);

//URL解码
uint8_t* UrlDecode(char* str);

//URL编码
uint8_t* UrlEncode(char* str);

//Base64编码，编码后比源长多1/3，如果源长不是3的倍数需补位
int8_t* base64_encode(uint8_t* bindata, uint binlength, int8_t* base64);

//Base64解码
int base64_decode(int8_t* base64, uint8_t* bindata);

#endif
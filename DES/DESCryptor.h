#ifndef __DESC_H__
#define __DESC_H__

#include "des.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//3des 加密算法   第一个参数为密码，第二个参数为明文，第三个参数为输出的密文，第四个参数为明文的字节数
void FinalD3desEncryption (int8_t * key , int8_t * inblock , int8_t * outbolck , uint32_t inlen);
//3des 解密算法   第一个参数为密码，第二个参数为密文，第三个参数为输出的明文，第四个参数为密文的字节数
void FinalD3desDecryption (int8_t * key , int8_t *inblock , int8_t * outbolck , uint32_t inlen);

#endif

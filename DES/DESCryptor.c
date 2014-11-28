#include "DESCryptor.h"

//3des 的解密算法第一个参数为密码，第二个参数为密钥，第三个参数为输出的明文，第四个参数为密钥的位数
void FinalD3desDecryption (uint8_t * key , uint8_t *inblock , uint8_t * outbolck , uint32_t srclen)
{
    DES3_CBC_CTX context;
    uint8_t iv[8] = {0};
    DES3_CBCInit(&context, key, iv, 0);
    DES3_CBCUpdate(&context, outbolck, inblock, srclen);
}

//3des 的加密算法第一个参数为密码，第二个参数为明文，第三个参数为输出的密文，第四个参数为明文的位数
void FinalD3desEncryption (uint8_t * key , uint8_t *inblock , uint8_t * outbolck , uint32_t srclen)
{
    // 3DES加密个长度关系为：原文以每8字节为段影响密文的最终长度，密文以8字节为步长，总长=8*原文段数
    int bAlloc = 0;
    if(srclen % 8)
    {
        bAlloc = 1;
        uint32_t arit = srclen % 8;
        uint32_t len = srclen + ((arit == 0) ? 0 : (8 - arit));
        //unsigned char *buff = new unsigned char[len];
        uint8_t *buff = (uint8_t *)malloc(len);
        //ZeroMemory(buff, len);
        memset(buff, 0, len);
        //memcpy_s(buff, len, inblock, srclen);
        memcpy(buff, inblock, srclen);
        inblock = buff;
        srclen = len;
    }

    DES3_CBC_CTX context;
    uint8_t iv[8] = {0};
    DES3_CBCInit(&context, key, iv, 1);
    DES3_CBCUpdate(&context, outbolck, inblock, srclen);

    if(bAlloc)
    {
        //delete inblock;
        free(inblock);
    }
}

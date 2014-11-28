#include "DESCryptor.h"

//3des �Ľ����㷨��һ������Ϊ���룬�ڶ�������Ϊ��Կ������������Ϊ��������ģ����ĸ�����Ϊ��Կ��λ��
void FinalD3desDecryption (uint8_t * key , uint8_t *inblock , uint8_t * outbolck , uint32_t srclen)
{
    DES3_CBC_CTX context;
    uint8_t iv[8] = {0};
    DES3_CBCInit(&context, key, iv, 0);
    DES3_CBCUpdate(&context, outbolck, inblock, srclen);
}

//3des �ļ����㷨��һ������Ϊ���룬�ڶ�������Ϊ���ģ�����������Ϊ��������ģ����ĸ�����Ϊ���ĵ�λ��
void FinalD3desEncryption (uint8_t * key , uint8_t *inblock , uint8_t * outbolck , uint32_t srclen)
{
    // 3DES���ܸ����ȹ�ϵΪ��ԭ����ÿ8�ֽ�Ϊ��Ӱ�����ĵ����ճ��ȣ�������8�ֽ�Ϊ�������ܳ�=8*ԭ�Ķ���
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

#ifndef __DESC_H__
#define __DESC_H__

#include "des.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//3des �����㷨   ��һ������Ϊ���룬�ڶ�������Ϊ���ģ�����������Ϊ��������ģ����ĸ�����Ϊ���ĵ��ֽ���
void FinalD3desEncryption (int8_t * key , int8_t * inblock , int8_t * outbolck , uint32_t inlen);
//3des �����㷨   ��һ������Ϊ���룬�ڶ�������Ϊ���ģ�����������Ϊ��������ģ����ĸ�����Ϊ���ĵ��ֽ���
void FinalD3desDecryption (int8_t * key , int8_t *inblock , int8_t * outbolck , uint32_t inlen);

#endif

#ifndef __CURL_H__
#define __CURL_H__

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "curl/curl.h"

#define BASIC "Basic"
#define BEARER "Bearer"

//接收数据的存放结构体,使用完数据后要free掉data
typedef struct 
{
	uint len;	//数据长度
	int8_t* data;	//数据存放地址
}ReturnData,*RetData;

static char* pCA=NULL;	//证书位置

//设置Http消息头
struct curl_slist* set_header(char* host,char* AuthType,char* Authorization);

void release_header(struct curl_slist *header);

/*
* @初始化curl库，需在主线程中使用
* @path_ca表示证书路径，不需要证书则为NULL
* @return 0为成功，-1为失败
*/
int curl_init(char* path_ca);

/*
* @HTTP POST请求
* @url为请求地址
* @postdata表示上传数据
* @postlen表示上传数据长度
* @data用于存储接收的数据
* @link_timeout表示连接超时时间，单位毫秒
* @ret_timeout表示接收超时时间，单位毫秒
* @headers表示自定义Http头
* @SSL表示是否使用SSL，非0为是，0为否，注意url需要使用https
* @return 0为成功，-1为失败
*/
int curl_post(char* url,int8_t* postdata,uint postlen,RetData data,uint link_timeout,uint ret_timeout,struct curl_slist *headers,int SSL);

/*
* @HTTP GET请求
* @url为请求地址
* @data用于存储接收的数据
* @link_timeout表示连接超时时间，单位毫秒
* @ret_timeout表示接收超时时间，单位毫秒
* @headers表示自定义Http头
* @SSL表示是否使用SSL，非0为是，0为否，注意url需要使用https
* @return 0为成功，-1为失败
*/
int curl_get(char* url,RetData data,uint link_timeout,uint ret_timeout,struct curl_slist *headers,int SSL);

/*
* @结束CURL库的调用，在主线程中执行 
*/
void curl_release();

#endif
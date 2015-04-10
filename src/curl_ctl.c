#include "curl_ctl.h"

/*
* @接收数据时的回调函数
* @ptr为数据缓冲区首地址
* @size为单数据块长度
* @nmemb为数据块数
* @stream为用于存储返回数据的地址
*/
static size_t writeData(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if(ptr==NULL||stream==NULL)
	{
		return -1;
	}
	size_t realsize = size * nmemb;
	RetData data=(RetData)stream;
	char* temp=(int8_t*)realloc(data->data,data->len+1+realsize);
	if(NULL==temp)
	{
		return -1;
	}
	data->data=temp;
	memset(data->data,0,data->len+1+realsize);
	memcpy(&(data->data[data->len]),ptr,realsize);
	data->len+=realsize;
	return realsize;
}

struct curl_slist* set_header(char* host,char* AuthType,char* Authorization)
{
	struct curl_slist* header=NULL;
	char auth[128]={0};
	char whost[128]={0};
	char contenttype[128]={0};
	sprintf(auth,"Authorization: %s %s", AuthType, Authorization);
	sprintf(whost,"Host: %s",host);
	sprintf(contenttype,"Content-Type: application/x-www-form-urlencoded");
	header=curl_slist_append(header, auth);
	header=curl_slist_append(header, whost);
	header=curl_slist_append(header, contenttype);
	return header;
}

int curl_init(char* path_ca)
{
	if(CURLE_OK!=curl_global_init(CURL_GLOBAL_DEFAULT))
	{
		return -1;
	}
	if(NULL!=path_ca)
	{
		pCA=path_ca;
	}
	return 0;
}

int curl_post(char* url,int8_t* postdata,uint postlen,RetData data,uint link_timeout,uint ret_timeout,struct curl_slist *headers,int SSL)
{
	int ret=-1;
	data->data=(int8_t*)malloc(1);
	if(NULL==data->data)
		return ret;
	data->len=0;
	CURL* curl=curl_easy_init();
	if(NULL==curl)
		return ret;
	//打印上传细节
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	//设置URL
	curl_easy_setopt(curl, CURLOPT_URL, url);
	//重定向跟随
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	//设置POST
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	//设置上传数据长度
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postlen);
	if(postlen>0)
	{
		//设置上传数据
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
	}
	//设置读取数据
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
	//设置存放返回数据的指针
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)data);
	//屏蔽中断信号
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//设置连接超时
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, link_timeout);
	//设置读取超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, ret_timeout);
	//设置自定义header
	if(NULL!=headers)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	//SSL设置
	if(SSL)
	{
		//判断证书
		if(NULL == pCA)
		{
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_CAINFO, pCA);
		}
	}
	//开始上传
	if(CURLE_OK==curl_easy_perform(curl))
	{
		//获取应答码
		int retcode;
		if(CURLE_OK == curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode))
		{
			ret=retcode;
		}
	}
	if(NULL!=headers)
	{
		curl_slist_free_all(headers);
	}
	curl_easy_cleanup(curl);
	return ret;
}

int curl_get(char* url,RetData data,uint link_timeout,uint ret_timeout,struct curl_slist *headers,int SSL)
{
	int ret=-1;
	data->data=(int8_t*)malloc(1);
	if(NULL==data->data)
		return ret;
	data->len=0;
	CURL* curl=curl_easy_init();
	if(NULL==curl)
		return ret;
	//打印上传细节
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	//设置URL
	curl_easy_setopt(curl, CURLOPT_URL, url);
	//重定向跟随
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	//设置读取数据
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
	//设置存放返回数据的指针
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)data);
	//屏蔽中断信号
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//设置连接超时
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, link_timeout);
	//设置读取超时
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, ret_timeout);
	//设置自定义header
	if(NULL!=headers)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	//SSL设置
	if(SSL)
	{
		//判断证书
		if(NULL == pCA)
		{
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_CAINFO, pCA);
		}
	}
	//开始请求
	if(CURLE_OK==curl_easy_perform(curl))
	{
		//获取应答码
		int retcode;
		if(CURLE_OK == curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode))
		{
			ret=retcode;
		}
	}
	if(NULL!=headers)
	{
		curl_slist_free_all(headers);
	}
	curl_easy_cleanup(curl);
	return ret;
}

void curl_release()
{
	curl_global_cleanup();
}
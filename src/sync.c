#include "head.h"

static struct evhttp *httpd=NULL;

void root_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf = evbuffer_new();
    if (buf == NULL)
	{
         printf("failed to create response buffer\n");
	}
	else
	{
		//Post内容处理
		/*char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
		if(post_data!=NULL)
		{
			evbuffer_add_printf(buf, "PostData: %s\n", post_data);
		}*/
		pthread_t pid;
		int flg=1;
		if(pthread_create(&pid,NULL,(void*)fresh_schema,(void*)(&flg))<0)
		{
			printf("pthread_create fresh_schema error\n");
			exit(-1);
		}
		MSG_INFO(CTRLPERSON);
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
	}
}

int init_listen_scheam(uint port,char* url)
{
	event_init();
	httpd = evhttp_start("0.0.0.0", port);
	if(NULL==httpd)
	{
		return -1;
	}
	evhttp_set_cb(httpd, url, root_handler, NULL);
	event_dispatch();
}

void exit_listen()
{
	evhttp_free(httpd);
	httpd=NULL;
}

void* listen_schema()
{
	pthread_detach(pthread_self()); //分离线程
	if(init_listen_scheam(HTTP_LISTEN_PORT,HTTP_LISTEN_URL)<0)	//初始化http监听
	{
		printf("init_listen_scheam error\n");
		pthread_exit((void*)-1);
	}
}
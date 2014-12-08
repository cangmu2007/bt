#include "log.h"

int openlog(char* path)
{
	if((fd=open(path,O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR))<0)
	{
		perror("open log error");
	}
	return fd;
}

static pthread_mutex_t mutex_log=PTHREAD_MUTEX_INITIALIZER;
int writelog(char* msg)
{
	pthread_mutex_lock(&mutex_log);
	char* time=gettime();
	if(NULL==time)
	{
		return -1;
	}
	uint len=strlen(msg)+strlen(time)+2;

	char* line=(char*)malloc(len+1);
	if(NULL==line)
	{
		return -1;
	}
	memset(line,0,len+1);

	strcat(line,time);
	line[26]=' ';
	strcat(line,msg);
	line[len-1]='\n';

	uint length=0;
	int ret=-1;
	while(length<len)
	{
		ret=write(fd,line+length,len-length);
		if(ret<0)
		{
			perror("write log error");
			break;
		}
		else
			length+=ret;
	}

	free(line);
	pthread_mutex_unlock(&mutex_log);
	return ret;
}

int closelog()
{
	close(fd);
}

char* gettime()
{
	time_t now;	//实例化time_t结构
	struct tm *timenow;	//实例化tm结构指针
	time(&now);
	//time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
	timenow=localtime(&now);

	static char tt[28];
	memset(tt,0,28);
	strcat(tt,"[");
	//localtime函数把从time取得的时间now换算成你电脑中的时间(就是你设置的地区)
	strcat(tt,asctime(timenow));
	tt[25]=']';

	return tt;
}
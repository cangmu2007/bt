#include "head.h"
#include "HMACSHA1.h"
#include <time.h>

//用户中心相关地址
#define BUS_TOK "/auth/token"
#define BUS_SCH "/resource/dept/get_dept_fk"
#define BUS_INFO "/resource/user/get_info"
#define BUS_MOD	"/resource/user/set_my_info"
#define BUS_RES "/resource/file/get?file_id="
#define BUS_MSG "/resource/msg/Receive2"

char* setUrl(char* url,int ssl,char* fun,char* other)
{
	if(ssl)
	{
		if(NULL==other)
			sprintf(url,"https://%s%s",USER_ADDR,fun);
		else
			sprintf(url,"https://%s%s%s",USER_ADDR,fun,other);
	}
	else
	{
		if(NULL==other)
			sprintf(url,"http://%s%s",USER_ADDR,fun);
		else
			sprintf(url,"http://%s%s%s",USER_ADDR,fun,other);
	}
	return url;
}

int setAuth(char* client_id,char* client_pass)
{
	if(NULL!=author)
	{
		memset(author,0,64);
	}

	char pwd[36]={0};
	if(md5_encode(client_pass,strlen(client_pass),pwd)<0)
	{
		printf("md5_encode error\n");
		return -1;
	}

	char auth[80]={0};
	strcat(auth,client_id);
	strcat(auth,pwd);

	char hsout[64]={0};
	int hsoutlen=64;

	if(HMACSHA1(clientkey,keylen,auth,strlen(auth),hsout,&hsoutlen)<0)
	{
		printf("HMACSHA1 error\n");
		return -1;
	}

	base64_encode(hsout,hsoutlen,author);
	return 0;
}

//keylen=base64_decode(key64, key);

int fresh_token(UL ul)
{	
	int res=-1,ret=-1;
	if(NULL==ul)
	{
		return res;
	}
	char data[256]={0};
	sprintf(data,"grant_type=refresh_token&refresh_token=%s",ul->refresh_token);
	struct curl_slist *headers=set_header(USER_ADDR,BASIC,author);
	ReturnData rd={0};
	char furl[128]={0};
	if((ret=curl_post(setUrl(furl,1,BUS_TOK,NULL),data,strlen(data),&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
	}
	else
	{
		if(200==ret)
		{
			ReToken retn={0};
			if(analysis_token(rd.data,&retn)<0)
			{
				goto _err;
			}
			memset(ul->access_token,0,36);
			memset(ul->refresh_token,0,36);
			strcpy(ul->access_token,retn.access_token);
			strcpy(ul->refresh_token,retn.refresh_token);
			res=0;
		}
		else if(400==ret)
		{
			char logmsg[32]={0};
			if(analysis_error(rd.data,logmsg)==0)
			{
				writelog(logmsg);
				res=-1;
			}
		}
	}
_err:
	free(rd.data);
	return res;
}

int web_check_up(char *uid,char *pass,Token retn)
{
	int ret=-1,res=-1;
	char data[256]={0};
	sprintf(data,"grant_type=password&username=%s&password=%s&client_id=%s&scope=manage",uid,pass,CLIENT_ID);
	struct curl_slist *headers=set_header(USER_ADDR,BASIC,author);
	ReturnData rd={0};
	char furl[128]={0};
	if((ret=curl_post(setUrl(furl,1,BUS_TOK,NULL),data,strlen(data),&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
	}
	else
	{
		if(200==ret)
		{
			if(analysis_token(rd.data,retn)<0)
			{
				goto _err;
			}
			res=1;
		}
		else if(400==ret)
		{
			char logmsg[32]={0};
			if(analysis_error(rd.data,logmsg)==0)
			{
				//用户名或密码错误
				if(strcmp(logmsg,ERROR_7)==0)
					res=0;
				else
					res=-1;
			}
		}
	}
_err:
	free(rd.data);
	return res;
}

char* web_get_schema(UL ul,int flag)
{
	if(NULL==ul)
	{
		return NULL;
	}
	struct curl_slist *headers=set_header(USER_ADDR,BEARER,ul->access_token);
	ReturnData rd={0};
	char furl[128]={0};
	int ret=-1;
	char* results=NULL;
	if((ret=curl_post(setUrl(furl,1,BUS_SCH,NULL),NULL,0,&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
	}
	else
	{
		if(200==ret)
		{
			results=analysis_schema(rd.data);
		}
		else if(400==ret&&0==flag)
		{
			int res=analysis_res_error(rd.data);
			if(10021==res)
			{
				if(fresh_token(ul)==0)
				{
					results=web_get_schema(ul,1);
				}
			}
		}
	}
	free(rd.data);
	return results;
}

char* web_get_info(UL ul,char* desid,int flag)
{
	struct curl_slist *headers=set_header(USER_ADDR,BEARER,ul->access_token);
	ReturnData rd={0};
	char furl[128]={0};
	int ret=-1;
	char data[64]={0};
	sprintf(data,"username=%s",desid);
	char* results=NULL;
	if((ret=curl_post(setUrl(furl,1,BUS_INFO,NULL),data,strlen(data),&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
	}
	else
	{
		if(200==ret)
		{
			UI ui=NULL;
			if(strcmp(ul->id,desid)==0)
			{
				ui=&(ul->usrinfo);
			}
			else
			{
				UserInfo uif={0};
				ui=&uif;
			}

			if(analysis_userinfo(rd.data,ui)<0)
			{
				goto _err;
			}
			if(get_sign(desid,ui->Sign)<0)
			{
				goto _err;
			}
			results=(char*)malloc(1024);
			memset(results,0,1024);
			strcat(results,MO_XML_HEAD);
			sprintf(results+strlen(results), \
				"<company><workphone>%s</workphone><mobilephone>%s</mobilephone><email>%s</email><photo>%s</photo><sign><![CDATA[%s]]></sign></company>", \
				ui->Phone,ui->Mobile,ui->Mail,ui->Photo,ui->Sign);
		}
		else if(400==ret&&0==flag)
		{
			int res=analysis_res_error(rd.data);
			if(10021==res)
			{
				if(fresh_token(ul)==0)
				{
					free(results);
					results=web_get_info(ul,desid,1);
				}
			}
		}
	}
_err:
	free(rd.data);
	return results;
}

int web_check_avatar(char* pid,char* md5val)
{
	int ret=-1;
	char furl[128]={0};
	ReturnData rd={0};
	if((ret=curl_get(setUrl(furl,1,BUS_RES,pid),&rd,30,30,NULL,0))<0)
	{
		printf("curl_get error\n");
	}
	else
	{
		if(200==ret)
		{
			char filemd5[36]={0};
			if(md5_encode(rd.data,rd.len,filemd5)<0)
			{
				ret=-1;
			}
			else
			{
				if(strcmp(filemd5,md5val)==0)
					ret=1;
				else
					ret=0;
			}
		}
		else
		{
			ret=-1;
		}
	}
	free(rd.data);
	return ret;
}

int web_updata_info(UL ul,char* Mood,char* Other)
{
	int ret=-1;
	if(NULL==ul)
	{
		return ret;
	}
	if(update_user_mood(ul->id,Mood)<0)
	{
		return ret;
	}
	struct curl_slist *headers=set_header(USER_ADDR,BEARER,ul->access_token);
	ReturnData rd={0};
	char furl[128]={0};
	if((ret=curl_post(setUrl(furl,1,BUS_MOD,NULL),Other,strlen(Other),&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
	}
	else
	{
		if(200==ret)
		{
			ret=0;
		}
		else if(400==ret)
		{
			int res=analysis_res_error(rd.data);
			if(10021==res)
			{
				if(fresh_token(ul)==0)
				{
					ret=web_updata_info(ul,ul->id,0);
				}
			}
		}
	}
	free(rd.data);
	return ret;
}

void getParameter(char *out)
{
	time_t timer=time(NULL);
	timer=timer-604800; //60*60*24*7
	struct tm *tblock=localtime(&timer);
	sprintf(out,"from=%.2d%%2d%.2d%%2d%.2d+%.2d%%3a%.2d%%3a%.2d&num=100", (1900+tblock->tm_year),(1+tblock->tm_mon),tblock->tm_mday,tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
}

char* web_get_notify(UL ul,int flag)
{
	struct curl_slist *headers=NULL;
	ReturnData rd={0};
	char furl[128]={0};
	int ret=-1,res;
	char* result=(char*)malloc(512);
	if(NULL==result)
	{
		return NULL;
	}
	memset(result,0,512);

	char parameter[48]={0};
	headers=set_header(USER_ADDR,BEARER,ul->access_token);
	memset(parameter,0,48);
	getParameter(parameter);
	if((ret=curl_post(setUrl(furl,1,BUS_MSG,NULL),parameter,strlen(parameter),&rd,30,30,headers,1))<0)
	{
		printf("curl_post error\n");
		writelog("curl_post error");
		free(result);
		result=NULL;
	}
	else
	{
		if(200==ret)
		{
			if(NULL!=rd.data)
			{
				if(analysis_res_notify(rd.data,result)!=0)
				{
					free(result);
					result=NULL;
				}
			}
		}
		else if(400==ret&&flag)
		{
			res=analysis_res_error(rd.data);
			if(10021==res)
			{
				if(fresh_token(ul)<0)
				{
					free(result);
					result=NULL;
				}
				else
				{
					free(result);
					result=web_get_notify(ul,0);
				}
			}
			else
			{
				free(result);
				result=NULL;
			}
		}
		else
		{
			free(result);
			result=NULL;
		}	
	}
	free(rd.data);
	return result;
}

static pthread_mutex_t schema_fresh=PTHREAD_MUTEX_INITIALIZER;	//用户链表处理线程锁
void* fresh_schema(void* flg)
{
	int f=*((int*)flg);
	if(f)
		pthread_detach(pthread_self()); //分离线程
	pthread_mutex_lock(&schema_fresh);
	UL p=user->next;
	if(p)
	{
		fresh_org=0;
		char* stmp=web_get_schema(p,0);
		if(NULL!=stmp)
		{
			if(NULL!=org_stu)
			{
				free(org_stu);
				org_stu=NULL;
			}
			printf("fresh schema\n");
			writelog("fresh schema");
			org_stu=stmp;
		}
	}
	else
		fresh_org=1;
	pthread_mutex_unlock(&schema_fresh);
}

/*void* listen_schema()
{
	pthread_detach(pthread_self()); //分离线程
	int ret=-1;
	ReturnData rd;

	for(;;)
	{
		memset(&rd,0,sizeof(ReturnData));
		if((ret=curl_get(LISTEN_MSG_URL,&rd,30,240,NULL,0))>=0)
		{
			if(200==ret)
			{
				printf("fresh schema\n");
				writelog("fresh schema");
				fresh_schema();
				MSG_INFO(CTRLPERSON);
				sleep(30);
			}
		}
		if(NULL!=rd.data)
		{
			free(rd.data);
		}
	}
}*/

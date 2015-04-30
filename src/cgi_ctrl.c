#include "head.h"

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
		writelog("fcntl(sock,GETFL");
        exit(-1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
		writelog("fcntl(sock,SETFL,opts)");
        exit(-1);
    }
}

void* CGI_Link(void* nfd)   //CGI连接处理
{
	pthread_detach(pthread_self()); //分离线程
    int fd=*((int*)nfd);
	uint32_t size=0;
    int ret=-1;
    while(((ret=recv(fd,(char*)&size,sizeof(uint32_t),0))<0)&&(errno == EAGAIN||errno == EINTR));	//接收包的长度
	if(ret!=sizeof(uint32_t)||size<=sizeof(uint32_t))
    {
        perror("CGI recv2");
		writelog("CGI recv2");
    }
	else
	{
        uint leftsize = sizeof(char) * (size - sizeof(uint32_t));
        char *rbuf = (char *)malloc(leftsize);
		if(NULL==rbuf)
		{
			 perror("CGI malloc");
			 writelog("CGI malloc");
			 goto _re;
		}
        memset(rbuf,0,leftsize);
		uint recv_count=0;
		while(recv_count<leftsize)	//循环接收剩下部分
		{
			//while(((ret=recv(fd,rbuf+recv_count,leftsize-recv_count,0))<0)&&(EAGAIN == errno));
			ret=recv(fd,rbuf+recv_count,leftsize-recv_count,0);
			if(ret<0)
			{
				if(errno != EAGAIN&&errno != EINTR)	//如果出现EAGAIN，则忽略
				{
					perror("CGI recv3");
					writelog("CGI recv3");
					break;
				}
			}else if(ret==0)
			{
				break;
			}else
				recv_count+=ret;
		}
        if(recv_count<leftsize)	//长度不对
        {
			free(rbuf);
            printf("CGI recv error!\n");
			writelog("CGI recv error");
        }
        else
        {
            CM cgi_pk=(CM)malloc(sizeof(char) * size);
			if(NULL==cgi_pk)
			{
				free(rbuf);
				goto _re;
			}
            cgi_pk->packet_len=size;
            memcpy((char*)(&cgi_pk->type), rbuf, leftsize); 
			free(rbuf);

            //pthread_mutex_lock(&mutex_sql);
            CRM crm=Business_deal(cgi_pk,fd);	//实际业务处理
            //pthread_mutex_unlock(&mutex_sql);
            if((0==cgi_pk->type))	//0是特殊业务，需特殊处理
            {
                //服务器推送
                ret=1;
                free(cgi_pk);
                if(crm!=NULL)
                {
                    Zero_RE(fd,crm->context,crm->len,1);
                    free(crm);
                }
                pthread_exit((void*)&ret);
            }
            free(cgi_pk);

			if(NULL==crm)
			{
				goto _re;
			}

			uint sent=0;
			while(sent<crm->len+sizeof(uint32_t))
			{
				if((ret=send(fd,(char*)crm+sent,crm->len+sizeof(uint32_t)-sent,MSG_NOSIGNAL))<0)	//返回处理结果
				{
					if(errno == EINTR)
					{
						continue;
					}

					if(errno == EAGAIN)
					{
						usleep(1000);
						continue;
					}
					perror("CGI send");
					writelog("CGI send");
					break;
				}
				else if(ret==0)
				{
					break;
				}
				else
				{
					sent+=ret;
				}
			}
			if(NULL!=crm)
			{
				free(crm);
				crm=NULL;
			}
        }
    }
_re:
    close(fd);
    if(-1==ret)
        pthread_exit((void*)&ret);
    return NULL;
}

void* Check_OutLine(void* nfd)
{
    pthread_detach(pthread_self()); //分离线程
    User_Linking ul=*((UL)nfd);
    pthread_testcancel();
    sleep(240);
    pthread_testcancel();
    //用户掉线
    close(ul.fd);
    MS32CHARINFO ms32={0};
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_LOGOUT, REQUEST, NONE,32);
    strcpy(ms32.id,ul.id);
    MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
    UL ull=(UL)nfd;
    if(ull!=NULL&&memcmp(ull,&ul,sizeof(User_Linking))==0)
    {
        delete_point_log(user,ull);
        ull=NULL;
        flush_list(user);
    }
	return NULL;
}

int Zero_RE(int fd,char* context,int len,int type)
{
    int ret=-1;
    CRM crm=(CRM)malloc(len+sizeof(uint32_t)+1);
	if(NULL==crm)
	{
		return -1;
	}
    memset(crm,0,len+sizeof(uint32_t)+1);

    //查看有无新消息
    memcpy(crm->context,context,len);
    crm->len=len;
    pthread_mutex_lock(&mutex_cgi); //上锁
	uint sent=0;
	while(sent<crm->len+sizeof(uint32_t))
	{
		if((ret=send(fd,(char*)crm+sent,crm->len+sizeof(uint32_t)-sent,MSG_NOSIGNAL))<0)
		{
			if(errno == EINTR)
			{
				continue;
			}

			if(errno == EAGAIN)
			{
				usleep(1000);
				continue;
			}
			perror("CGI re send");
			writelog("CGI re send");
			break;
		}
		else if(ret==0)
		{
			break;
		}
		else
		{
			sent+=ret;
		}
	}
    pthread_mutex_unlock(&mutex_cgi); //解锁
	free(crm);
	crm=NULL;

    /*if(ret==-1) //出错
    {
        free(crm);
        crm=NULL;
        goto _ret;
    }
    free(crm);
    crm=NULL;*/

    /*if(type==1)
    {
        pthread_t time=0;
        UL ul=get_point_fd(user,fd);
        if((ret=pthread_create(&time,NULL,(void*)Check_OutLine,(void*)ul))>-1) //90s计时
        {
            if((ret=update_point_pth(user,fd,time))>-1)
            {
                for(;;)
                {
                    kill=pthread_kill(time,0);
                    if(ESRCH!=kill&&EINVAL!=kill)
                        break;
                    usleep(300);    //等待线程启动
                }
            }
        }
    }*/
//_ret:
    if(type==0)
        printf("close fd=%d,result=%d\n",fd,close(fd));
    return ret;
}

void* Flush_CGI(void* arg)
{
	pthread_detach(pthread_self()); //分离线程
    sleep(3);
    for(;;)
    {
        printf("Keep-Aliving\n");
        if(get_len(user)>0)
        {
            UL p=user->next;
            while(p)
            {
                if(-1!=p->fd&&p->flag==1)
                {
                    Zero_RE(p->fd,"null",4,1);
                    p->flag=0;
                }
                p=p->next;
            }
        }
        sleep(240);
    }
}

void cgi_all_send(char* msg,char* me)
{
    UL p=user->next;
    uint len=strlen(msg);
    while(p)
    {
        if(strcmp(me,p->id)!=0)
        {
            if(p->fd!=-1)
            {
                if(p->flag==1)  //1表示可以发送，0表示不能发送
                {
                    Zero_RE(p->fd,msg,len,1);
                    p->flag=0;
                }
                else
                    insert_imf(p->il,msg,len);  //不能发送则加入通知队列
            }
        }
        p=p->next;
    }
}


CRM Business_deal(CM cm,int fd)
{
    CRM crm=NULL;
    char* text=NULL;
    int flag=0;
    switch(cm->type)
    {
        case BUS_SERVER_PUSH:	//用户请求消息
            if(NULL==(text=Check_Logined(cm->sender)))
            {
                UL ul=get_point(user,cm->sender);
                text=ServerPush(cm->sender,cm->context,fd,ul);
                if(text==NULL)
                    ul->flag=1;
            }
			if(NULL==text||0==strcmp(text,"FAULT")||0==strcmp(text,"nologin")||0==strcmp(text,"update 5")||0==strcmp(text,"update 8")||0==strcmp(text,"update 12"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
	/*
	特别记录，0业务的text有如下返回值：
	1.nologin，表示用户未登录
	2.FAULT，表示执行失败
	3.5，表示通知用户更新组织结构(需free)
	4.8，表示通知用户更新群列表(需free)
	5.12，表示通知用户更新多人会话列表(需free)
	6.消息（gzip压缩的二进制数据）
	  （1）前8byte为数字字符串，表示长度
	  （2）2byte均为\0（万恶的Sql Server的0x标识符(╯‵□′)╯︵┻━┻_(:з」∠)_）
	  （3）二进制压缩数据
	7.其它，都是出错
	*/
		
        case BUS_LOGIN:	//用户登录
            text=Login(cm->sender,cm->context);
            break;
        case BUS_CANCELLATION:	//用户注销
            text=Cancellation(cm->sender);
            break;
        case BUS_INDIVIDUAL_SESSION:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=Talk(CTRLPERSON,cm->sender,cm->recver,cm->context,cm->len,cm->dev_type);
            break;
        case BUS_GROUP_SESSION:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=Talk(CTRLGROUP,cm->sender,cm->recver,cm->context,cm->len,cm->dev_type);
            break;
        case BUS_MULTIPLAYER_SESSION:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=Talk(CTRLMUTIL,cm->sender,cm->recver,cm->context,cm->len,cm->dev_type);
            break;
        case BUS_OL_MOBILE:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetMo_Ol();
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
        case BUS_OL_PC:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetPc_Ol();
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
        case BUS_ORG_STU:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetOrg_Stu();
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
        case BUS_GROUPER:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetGrouper(cm->context);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_GROUP_SHIELD:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=SetShield(cm->sender,cm->context);
            break;
        case BUS_GROUP:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetGroup(cm->sender);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_AVATAR:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetAvatar(cm->context);
            if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
        case BUS_LOGINER_MSG:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetOrger_Msg(cm->sender,cm->context);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_MULTIPLAYER:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetMultiplayer(cm->context);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_MULTI:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetMulti(cm->sender);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_NEW_GROUP:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=NewGroup(cm->sender,cm->context);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_NEW_MULTIPLAYER_SESSION:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=NewMulti(cm->sender,cm->context);
			if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            //flag=1;
            break;
        case BUS_ADD_MULTIPLAYER_SESSION:
            text=AddMulti(cm->context);
            break;
        case BUS_EXIT_GROUP:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=ExitGroup(cm->sender,cm->context);
            flag=1;
            break;
        case BUS_EXIT_MULTI:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=ExitMulti(cm->sender,cm->context);
            flag=1;
            break;
        case BUS_UPDATE_LOGINER_MSG:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=UpdateLoginerMsg(cm->sender,cm->context);
            break;
        case BUS_CON_PIC:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetPicture(cm->context);
            if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
		case BUS_CHE_PHOTO:
			if(NULL==(text=Check_Logined(cm->sender)))
                text=Check_Photo(cm->context);
			break;
		case BUS_CHE_NOT:
			if(NULL==(text=Check_Logined(cm->sender)))
                text=Check_fresh_notify(cm->sender);
			if(0==strcmp(text,"FAULT")||0==strcmp(text,"null"))
                flag=1;
            else
                flag=2;
			break;
		case BUS_CON_SUD:
            if(NULL==(text=Check_Logined(cm->sender)))
                text=GetSound(cm->context);
            if(0==strcmp(text,"FAULT"))
                flag=1;
            else
                flag=2;
            break;
		/*case BUS_CIMS_ID:
			if(NULL==(text=Check_Logined(cm->sender)))
                text=Get_CIMS_ID(cm->sender);
			flag=1;
			break;*/
    }
    if(NULL!=text)
    {
        uint len=0;
        if(flag!=2)
        {
			uint datalen=strlen(text);
            len=datalen+sizeof(uint32_t)+1;
            crm=(CRM)malloc(len);
			if(NULL==crm)
			{
				return NULL;
			}
            memset(crm,0,len);
            memcpy(crm->context,text,datalen);
            crm->len=datalen;
        }
        else
        {
			uint datalen=atoi(text);
			if(datalen>0)
			{
				len=datalen+sizeof(uint32_t)+9;
				crm=(CRM)malloc(len);
				if(NULL==crm)
				{
					return NULL;
				}
				memset(crm,0,len);
				memcpy(crm->context,text+10,datalen);
				crm->len=datalen;
			}
        }
        if(flag==1&&0!=strcmp(text,"nologin")&&0!=strcmp(text,"FAULT")&&0!=strcmp(text,"OK")&&0!=strcmp(text,"login_error")&&0!=strcmp(text,"null"))
            free(text);
    }
    return crm;
}

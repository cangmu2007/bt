#include "head.h"
#include "compress.h"

char* Check_Logined(char* loginer)
{
    if(NULL==get_point(user,loginer))
        return "nologin";
    return NULL;
}

char* Login(char* loginer,char* password)
{
    if(NULL!=get_point(user,loginer))
        return loginer;
	int ret=check_up(loginer,password);
    if(ret==0)
		return "login_error";
	else if(ret==-1)
		return "FAULT";
    if(insert_point(user,loginer,-1)<0)
        return "FAULT";
    MS32CHARINFO ms32={0};
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_LOGIN, REQUEST, NONE,32);
    strcpy(ms32.id,loginer);
    if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
    {
		UL ul=get_point(user,loginer);
		if(NULL!=ul)
        	delete_point_log(user,ul);
		return "FAULT";
	}
    flush_list(user);
	//不再立刻通知客户端10-10
    //cgi_all_send("41",loginer);
    return loginer;
}

char* Cancellation(char* loginer)
{
    UL ul=get_point(user,loginer);
    if(NULL==ul)
        return "FAULT";
	int fd=ul->fd;
    if(-1==delete_point_log(user,ul))
        return "FAULT";
    Zero_RE(fd,"CANCEL",6,0);
    MS32CHARINFO ms32={0};
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_LOGOUT, REQUEST, NONE,32);
    strcpy(ms32.id,loginer);
    MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
    flush_list(user);
	//不再立刻通知客户端10-10
    //cgi_all_send("41",loginer);
    return "OK";
}

char* GetOrg_Stu()
{
    //if(get_org_stu(CONPANY_ID)<0)   //获取组织结构
	if(GetOnlineCtms(CONPANY_ID)<0)
    {
        printf("Failed to get the organizational structure\n");
		writelog("Failed to get the organizational structure");
		if(org_stu != NULL && 0 != strcmp(org_stu, MO_XML_HEAD))
		{
			free(org_stu);
		}
        org_stu=MO_XML_HEAD;
		return "FALUT";
        //exit(-1);
    }
	return xml_compress(org_stu,0);
    //return org_stu;
}

char* GetPc_Ol()
{
	return xml_compress(PC_OL,0);
    //return PC_OL;
}

char* GetMo_Ol()
{
	return xml_compress(MO_OL,0);
    //return MO_OL;
}

char* ServerPush(char* loginer,char* msg,int fd,UL ul)
{
    //根据msg修改数据库记录
    char* tmp=NULL;
    char ids[2048]= {0};
    if(0==strcmp(msg,"OK"))
    {
        if(ul->il!=NULL)
        {
            IL il=get_imf(ul->il);
            if(il!=NULL)
                delete_imf(ul->il,il);
        }
    }
    else if(0!=strcmp(msg,"null"))
    {
        if(NULL!=(tmp=strstr(msg,"type=1")))
        {
            if(sscanf(tmp+10,"%[^;]",ids)==1)
                delete_msg(ids,CTRLPERSON);
            }
			memset(ids,0,2048);
        if(NULL!=(tmp=strstr(msg,"type=2")))
        {
            if(sscanf(tmp+10,"%[^;]",ids)==1)
                delete_msg(ids,CTRLGROUP);
            }
			memset(ids,0,2048);
        if(NULL!=(tmp=strstr(msg,"type=3")))
        {
            if(sscanf(tmp+10,"%[^;]",ids)==1)
                delete_msg(ids,CTRLMUTIL);
            }
			memset(ids,0,2048);
    }
    if(update_point_fd(user,loginer,fd)<0)
        return "FAULT";
    char* re_msg=GetImf(ul,loginer);
	if(re_msg!=NULL)
	{
		return re_msg;
	}
	else
	{
		//查数据库里的离线信息
		re_msg=search_info(loginer,CTRLALL);
		if(re_msg!=NULL)
		{
			return xml_compress(re_msg,1);
		}
		else
		{
			return NULL;
		}
	}
}

char* GetImf(UL ul,char* loginer)
{
    if(ul->il==NULL)
        return "FAULT";
    IL il=get_imf(ul->il);
    if(il==NULL)
    {
        ul->flag=1;
        return NULL;
    }
    char* re_msg=(char*)malloc(il->len);
	if(NULL==re_msg)
	{
		return "FAULT";
	}
    memcpy(re_msg,il->context,il->len);
    return re_msg;
}

char* GetGrouper(char* gid)
{
    char* re_msg=get_member(atoi(gid),CTRLGROUP);
	if(NULL==re_msg)
		return "FAULT";
	else
		return xml_compress(re_msg,1);
}

char* SetShield(char* loginer,char* context)
{
    int gid,type;
    if(sscanf(context,"%d|%d",&gid,&type)!=2)
        return "FAULT";
    if(-1==set_group(loginer,gid,type))
    {
        return "FAULT";
    }
    MSGROUPCTRL mgc={0};
    strcpy(mgc.uid,loginer);
    mgc.gid=gid;
    mgc.opertype=type;
    mgc.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_NOTIFY, REQUEST, NONE,sizeof(MSGROUP));
    if(MI_Write((char*)&mgc,sizeof(MSGROUPCTRL),1)<0)
        return "FAULT";
    return "OK";
}

char* GetGroup(char* loginer)
{
    char* result=get_group_or_mutil(loginer,CTRLGROUP);
    if(NULL==result)
        return "FAULT";
	else
		return xml_compress(result,1);
}

char* GetAvatar(char* orger)
{
    char* photo=get_photo(orger);
	if(NULL==photo)
		return "FAULT";
	else
		return photo;
    //return get_photo(orger);
}

char* GetOrger_Msg(char* orger)
{
    char* result=get_info(orger);
	if(NULL==result)
		return "FAULT";
	else
		return xml_compress(result,1);
}

char* GetMultiplayer(char* mid)
{
    char* re_msg=get_member(atoi(mid),CTRLMUTIL);
	if(NULL==re_msg)
		return "FAULT";
	else
		return xml_compress(re_msg,1);
}

char* GetMulti(char* loginer)
{
    char* result=get_group_or_mutil(loginer,CTRLMUTIL);
    if(NULL==result)
        return "FAULT";
	else
		return xml_compress(result,1);
}

char* NewGroup(char* loginer,char* context)
{
    //context分割
    //unsigned char* data=UrlDecode(context);
    char name[32]= {0},theme[128]= {0},id[512]= {0};
    if(sscanf(context,"%*[^=]=%[^;]%*[^=]=%[^;]%*[^=]=%s",name,theme,id)!=3)
        return "FAULT";
    //free(data);
    MSGROUP mr= {0};
    mr.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_CREATE, REQUEST, NONE,sizeof(MSGROUP));
	strcpy(mr.uid,loginer);
    if(insert_group(loginer,name,theme,id,&mr.gid,CTRLGROUP)==-1) //数据库操作
	{
		return "FAULT";
	}
    if(MI_Write((char*)&mr,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}
    char tmp[32]= {0};
    uint len=0;
    while(sscanf(id+len,"%[^|]",tmp)==1)
    {
		if(0==strcmp(tmp,loginer))
		{
			break;
		}
        UL usr=get_point(user,tmp);
        if(NULL!=usr)
            if(usr->fd!=-1)
            {
				char* con="update 8";
				if(usr->flag==1)
                {
                    Zero_RE(usr->fd,con,strlen(con),1);
                    usr->flag=0;
                }
                else
                    insert_imf(usr->il,con,strlen(con));
            }
        len+=strlen(tmp)+1;
        memset(tmp,0,32);
    }
    return GetGroup(loginer);
}

char* NewMulti(char* loginer,char* context)
{
    //context分割
    //unsigned char* data=UrlDecode(context);
    char theme[128]= {0},id[512]= {0};
    if(sscanf(context,"%*[^=]=%[^;]%*[^=]=%s",theme,id)!=2)
        return "FAULT";
    //free(data);
    MSGROUP mr= {0};
    mr.bp=BsnsPacket_init(MC_BTANDRIOD_MULTI_CREATE, REQUEST, NONE,sizeof(MSGROUP));
	strcpy(mr.uid,loginer);
    if(insert_group(loginer,NULL,theme,id,&mr.gid,CTRLMUTIL)==-1)  //数据库操作
	{
		return "FAULT";
	}

    if(MI_Write((char*)&mr,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}

    char tmp[32]= {0};
    uint len=0;
    while(sscanf(id+len,"%[^|]",tmp)==1)
    {
		if(0==strcmp(tmp,loginer))
		{
			break;
		}
        UL usr=get_point(user,tmp);
        if(NULL!=usr)
            if(usr->fd!=-1)
            {
				char* con="update 12";
                if(usr->flag==1)
                {
                    Zero_RE(usr->fd,con,strlen(con),1);
                    usr->flag=0;
                }
                else
                    insert_imf(usr->il,con,strlen(con));
            }
        len+=strlen(tmp)+1;
        memset(tmp,0,32);
    }
    return GetMulti(loginer);
}

char* AddMulti(char* context)
{
    int ret=-1;
    uint len=strlen(context);
    MSEXTAPP_REG_REQ mm= (MSEXTAPP_REG_REQ)malloc(sizeof(MSEXTAPPREG_REQ)+len);
	if(NULL==mm)
	{
		return "FAULT";
	}
    memset(mm,0,sizeof(MSEXTAPPREG_REQ)+len);
    mm->bp=BsnsPacket_init(MC_BTANDRIOD_MULTI_ADDUSER, REQUEST, NONE,sizeof(uint32_t)+len);
    if(sscanf(context,"%*[^=]=%d;%*[^=]=%s",mm->ntype,mm->context)!=2)
        return "FAULT";
    if(insert_mutil_orger(mm->ntype,mm->context)==-1)
		return "FAULT";

    ret=MI_Write((char*)mm,sizeof(MSEXTAPPREG_REQ)+len,1);
    free(mm);
    if(-1==ret)
        return "FAULT";
    return "OK";
}

char* ExitGroup(char* loginer,char* gid)
{
    if(exit_group_mutil(loginer,atoi(gid),CTRLGROUP)==-1)
		return "FAULT";

    MSGROUP mg= {0};
    strcpy(mg.uid,loginer);
    mg.gid=atoi(gid);
    mg.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_DELUSER, REQUEST, NONE,sizeof(MSGROUP));
    if(MI_Write((char*)&mg,sizeof(MSGROUP),1)<0)
        return "FAULT";
    return  GetGroup(loginer);
}

char* ExitMulti(char* loginer,char* mid)
{
    if(exit_group_mutil(loginer,atoi(mid),CTRLMUTIL)==-1)
		return "FAULT";

    MSGROUP mg= {0};
    strcpy(mg.uid,loginer);
    mg.gid=atoi(mid);
    mg.bp=BsnsPacket_init(MC_BTANDRIOD_MULTI_DELUSER, REQUEST, NONE,sizeof(MSGROUP));
    if(MI_Write((char*)&mg,sizeof(MSGROUP),1)<0)
        return "FAULT";
    return GetMulti(loginer);
}

char* GetPicture(char* pid)
{
    int mpid;
	static pthread_mutex_t pmt=PTHREAD_MUTEX_INITIALIZER;
    char ext[16]= {0};
	uint8_t* pic=NULL;
    if(sscanf(pid,"%d.%s",&mpid,ext)==2)
	{
		pthread_mutex_lock(&pmt);
        pic=get_picture(mpid,ext);
		pthread_mutex_unlock(&pmt);
	}
	if(NULL==pic)
		return "FAULT";
	else
		return pic;
}

char* Talk(int type,char* src,char* des,char* context,uint32_t llen,uint32_t systype)
{
    int ret=-1,num=-1,t;
    if((t=insert_talklist(src,des,context,llen,type,systype))<0)
		return "FAULT";
    if(t==0)
    {
        if(CTRLPERSON==type)  //个人会话
        {
            MS64_CHARINFO ms64=(MS64_CHARINFO)malloc(sizeof(MS64CHARINFO)+llen);
			if(NULL==ms64)
			{
				return "FAULT";
			}
            memset(ms64,0,sizeof(MS64CHARINFO)+llen);
			//ms64->sys_type=systype;
            strcpy(ms64->srcid,src);
            strcpy(ms64->desid,des);
            memcpy(ms64->context,context,llen);
            ms64->bp=BsnsPacket_init(MC_BTANDRIOD_PTOP_MSG, REQUEST, NONE,64+llen);
            ret=MI_Write((char*)ms64,sizeof(MS64CHARINFO)+llen,1);
            num=MSG_RECV(RESPONSE,(char*)ms64->srcid,64,type);
            free(ms64);
        }
        else    //群/多人会话
        {
            MS_GROUP mg= (MS_GROUP)malloc(sizeof(MSGROUP)+llen);
			if(NULL==mg)
			{
				return "FAULT";
			}
            memset(mg,0,sizeof(MSGROUP)+llen);
			//mg->sys_type=systype;
            strcpy(mg->uid,src);
            mg->gid=atoi(des);
            memcpy(mg->context,context,llen);
            mg->bp=BsnsPacket_init((type==2)?MC_BTANDRIOD_GROUP_MSG:MC_BTANDRIOD_MULTI_MSG, REQUEST, NONE,32+sizeof(uint32_t)+llen);
            ret=MI_Write((char*)mg,sizeof(MSGROUP)+llen,1);
            num=MSG_RECV(RESPONSE,(char*)mg->uid,32+sizeof(uint32_t),type);
            free(mg);
        }
    }
    if((t==0)&&(-1==ret||-1==num))
        return "FAULT";
    return "OK";
}

char* UpdateLoginerMsg(char* loginer,char* context)
{
    //修改数据库
    char Mobile[48]= {0};
    char Phone[48]= {0};
    char Mail[48]= {0};
    char Mood[384]= {0};
    //char* data=UrlDecode(context);
    //sscanf(data,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    //free(data);
	sscanf(context,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    if(update_org_info(loginer,Phone,Mobile,Mail,Mood)==-1)
		return "FAULT";
    MS32CHARINFO ms32= {0};
	strcpy(ms32.id,loginer);
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_CTM_BASEINFO, REQUEST, NONE,32);
    if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
        return "FAULT";
	ms32.bp=BsnsPacket_init(MC_BTANDRIOD_CTM_MOOD, REQUEST, NONE,32);
	if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
        return "FAULT";
    return "OK";
}

char* Check_Photo(char* uid,char* md5)
{
	int ret=check_user_photo(uid,md5);
	if(ret==0)
		return "OK";
	else
		return "FAULT";
}

char* Get_CIMS_ID(char* uid)
{
	char* ret=getCIMS_id(uid);
	if(ret!=NULL)
		return ret;
	else
		return "FAULT";
}

/*void strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl)
{
    char* pi = pSrcIn;
    char* po = pDstOut;

    int nSrcRplLen = strlen(pSrcRpl);
    int nDstRplLen = strlen(pDstRpl);

    char *p = NULL;
    int nLen = 0;

    do
    {
        // 找到下一个替换点
        p = strstr(pi, pSrcRpl);

        if(p != NULL)
        {
            // 拷贝上一个替换点和下一个替换点中间的字符串
            nLen = p - pi;
            memcpy(po, pi, nLen);
            // 拷贝需要替换的字符串
            memcpy(po + nLen, pDstRpl, nDstRplLen);
        }
        else
        {
            strcpy(po, pi);
            // 如果没有需要拷贝的字符串,说明循环应该结束
            break;
        }
        pi = p + nSrcRplLen;
        po = po + nLen + nDstRplLen;
    }
    while (p != NULL);
}

int Count(char *const a,char *const b)
{
    //a为主串，b为子串
    char *p=a,*q=b;
    int count=0;
    while(*p)
    {
        if ((*p==*q)&&(*q)) //匹配条件
        {
            p++;
            q++;
        }
        else //不匹配q要重置，并在主串中继续查找
        {
            p++;
            q=b;
        }
        if (!(*q)) //匹配后q要重置
        {
            count++;
            q=b;
        }
    }
    return count;
}*/

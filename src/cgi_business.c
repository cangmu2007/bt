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
	{
        return loginer;
	}
	ReToken retn={0};
	int ret=web_check_up(loginer,password,&retn);
	if(ret==0)
		return "login_error";
	else if(ret==-1)
		return "FAULT";
	MS32CHARINFO ms32={0};
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_LOGIN, REQUEST, NONE,48);
    strcpy(ms32.id,loginer);
    if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
    {
		return "FAULT";
	}
	//MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
	UL ul=insert_point(user,loginer,-1);
	if(NULL==ul)
	{
        return "FAULT";
	}
	strcpy(ul->access_token,retn.access_token);
	strcpy(ul->refresh_token,retn.refresh_token);
    flush_list(user);
	//��������֪ͨ�ͻ���10-10
    //cgi_all_send("41",loginer);
    return loginer;
}

/*char* Login(char* loginer,char* password)
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
	//��������֪ͨ�ͻ���10-10
    //cgi_all_send("41",loginer);
    return loginer;
}*/

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
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_LOGOUT, REQUEST, NONE,48);
    strcpy(ms32.id,loginer);
    MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
    flush_list(user);
	//��������֪ͨ�ͻ���10-10
    //cgi_all_send("41",loginer);
    return "OK";
}

char* GetOrg_Stu()
{
	if(1==fresh_org||NULL==org_stu)
	{
		fresh_schema((void*)0);
		if(NULL==org_stu)
		{
			return "FAULT";
		}
	}
	return xml_compress(org_stu,0);
}

/*char* GetOrg_Stu()
{
    //if(get_org_stu(CONPANY_ID)<0)   //��ȡ��֯�ṹ
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
}*/

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
    //����msg�޸����ݿ��¼
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
		//�����ݿ����������Ϣ
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
    mgc.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_NOTIFY, REQUEST, NONE,48+sizeof(uint32_t));
    if(MI_Write((char*)&mgc,sizeof(MSGROUPCTRL),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&mgc,sizeof(MSGROUPCTRL),1)
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

char* GetOrger_Msg(char* loginer,char* orger)
{
	UL ul=get_point(user,loginer);
	if(NULL==ul)
	{
		return "FAULT";
	}
	char* result=web_get_info(ul,orger,0);
	if(NULL==result)
		return "FAULT";
	else
		return xml_compress(result,1);
}

/*char* GetOrger_Msg(char* orger)
{
    char* result=get_info(orger);
	if(NULL==result)
		return "FAULT";
	else
		return xml_compress(result,1);
}*/

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
    //context�ָ�
    //unsigned char* data=UrlDecode(context);
    char name[48]= {0},theme[128]= {0},id[512]= {0};
    if(sscanf(context,"%*[^=]=%[^;]%*[^=]=%[^;]%*[^=]=%s",name,theme,id)!=3)
        return "FAULT";
    //free(data);
    MSGROUP mr= {0};
    mr.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_CREATE, REQUEST, NONE,48+sizeof(uint32_t));
	strcpy(mr.uid,loginer);
    if(insert_group(loginer,name,theme,id,&mr.gid,CTRLGROUP)==-1) //���ݿ����
	{
		return "FAULT";
	}
    if(MI_Write((char*)&mr,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&mr,sizeof(MSGROUP),1)
    char tmp[48]= {0};
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
        memset(tmp,0,48);
    }
    return GetGroup(loginer);
}

char* NewMulti(char* loginer,char* context)
{
    //context�ָ�
    //unsigned char* data=UrlDecode(context);
    char theme[128]= {0},id[512]= {0};
    if(sscanf(context,"%*[^=]=%[^;]%*[^=]=%s",theme,id)!=2)
        return "FAULT";
    //free(data);
    MSGROUP mr= {0};
    mr.bp=BsnsPacket_init(MC_BTANDRIOD_MULTI_CREATE, REQUEST, NONE,48+sizeof(uint32_t));
	strcpy(mr.uid,loginer);
    if(insert_group(loginer,NULL,theme,id,&mr.gid,CTRLMUTIL)==-1)  //���ݿ����
	{
		return "FAULT";
	}

    if(MI_Write((char*)&mr,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&mr,sizeof(MSGROUP),1)
    char tmp[48]= {0};
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
        memset(tmp,0,48);
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
	{
        return "FAULT";
	}
    return "OK";
}

char* ExitGroup(char* loginer,char* gid)
{
    if(exit_group_mutil(loginer,atoi(gid),CTRLGROUP)==-1)
		return "FAULT";

    MSGROUP mg= {0};
    strcpy(mg.uid,loginer);
    mg.gid=atoi(gid);
    mg.bp=BsnsPacket_init(MC_BTANDRIOD_GROUP_DELUSER, REQUEST, NONE,48+sizeof(uint32_t));
    if(MI_Write((char*)&mg,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&mg,sizeof(MSGROUP),1);
    return  GetGroup(loginer);
}

char* ExitMulti(char* loginer,char* mid)
{
    if(exit_group_mutil(loginer,atoi(mid),CTRLMUTIL)==-1)
		return "FAULT";

    MSGROUP mg= {0};
    strcpy(mg.uid,loginer);
    mg.gid=atoi(mid);
    mg.bp=BsnsPacket_init(MC_BTANDRIOD_MULTI_DELUSER, REQUEST, NONE,48+sizeof(uint32_t));
    if(MI_Write((char*)&mg,sizeof(MSGROUP),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&mg,sizeof(MSGROUP),1);
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

char* GetSound(char* sid)
{
    int msid;
	static pthread_mutex_t smt=PTHREAD_MUTEX_INITIALIZER;
    char ext[16]= {0};
	uint8_t* sud=NULL;
    if(sscanf(sid,"%d.%s",&msid,ext)==2)
	{
		pthread_mutex_lock(&smt);
        sud=get_sound(msid,ext);
		pthread_mutex_unlock(&smt);
	}
	if(NULL==sud)
		return "FAULT";
	else
		return sud;
}

char* Talk(int type,char* src,char* des,char* context,uint32_t llen,uint32_t systype)
{
    int ret=-1,num=-1,t;
    if((t=insert_talklist(src,des,context,llen,type,systype))<0)
		return "FAULT";
    if(t==0)
    {
        if(CTRLPERSON==type)  //���˻Ự
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
            ms64->bp=BsnsPacket_init(MC_BTANDRIOD_PTOP_MSG, REQUEST, NONE,48+48+llen);
            ret=MI_Write((char*)ms64,sizeof(MS64CHARINFO)+llen,1);
            num=MSG_RECV((char*)ms64->srcid,48+48,type);
            free(ms64);
        }
        else    //Ⱥ/���˻Ự
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
            mg->bp=BsnsPacket_init((type==2)?MC_BTANDRIOD_GROUP_MSG:MC_BTANDRIOD_MULTI_MSG, REQUEST, NONE,48+sizeof(uint32_t)+llen);
            ret=MI_Write((char*)mg,sizeof(MSGROUP)+llen,1);
            num=MSG_RECV((char*)mg->uid,48+sizeof(uint32_t),type);
            free(mg);
        }
    }
	//if(t==0&&-1==num)
    if((t==0)&&(-1==ret||-1==num))
        return "FAULT";
    return "OK";
}

char* UpdateLoginerMsg(char* loginer,char* context)
{
    //�޸����ݿ�
    char Mobile[48]= {0};
    char Phone[48]= {0};
    char Mail[48]= {0};
    char Mood[384]= {0};
	char Uname[48]={0};
    //char* data=UrlDecode(context);
    //sscanf(data,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    //free(data);
	sscanf(context,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    
	UL ul=get_point(user,loginer);
	if(NULL==ul)
		return "FAULT";

	char Other[1024]={0};
	char tmp[512]={0};
	sprintf(tmp,"nick_name=%s&sex=%d&email_address=%s&phone_number=%s&cell_phone_number=%s", \
		ul->usrinfo.Nickname,ul->usrinfo.sex,Mail,Phone,Mobile);
	strcat(Other,tmp);
	if(NULL!=ul->usrinfo.real_name)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&real_name=%s",ul->usrinfo.real_name);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.personal_web_uri)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&personal_web_uri=%s",ul->usrinfo.personal_web_uri);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.birthday)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&birthday=%s",ul->usrinfo.birthday);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.qq_number)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&qq_number=%s",ul->usrinfo.qq_number);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.main_post)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&main_post=%s",ul->usrinfo.main_post);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.posts)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&posts=%s",ul->usrinfo.posts);
		strcat(Other,tmp);
	}
	if(NULL!=ul->usrinfo.Photo)
	{
		memset(tmp,0,512);
		sprintf(tmp,"&photo=%s",ul->usrinfo.Photo);
		strcat(Other,tmp);
	}
	if(web_updata_info(ul,Mood,Other)<0)
		return "FAULT";
    MS32CHARINFO ms32= {0};
	strcpy(ms32.id,loginer);
    ms32.bp=BsnsPacket_init(MC_BTANDRIOD_CTM_BASEINFO, REQUEST, NONE,48);
    if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
	ms32.bp=BsnsPacket_init(MC_BTANDRIOD_CTM_MOOD, REQUEST, NONE,48);
	if(MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1)<0)
	{
        return "FAULT";
	}
	//MI_Write((char*)&ms32,sizeof(MS32CHARINFO),1);
    return "OK";
}

char* Check_fresh_notify(char* loginer)
{
	UL ul=get_point(user,loginer);
	if(ul==NULL)
		return "FAULT";
	char* result=web_get_notify(ul,1);
	if(result==NULL)
		return "null";
	else
		return xml_compress(result,1);
}

/*char* UpdateLoginerMsg(char* loginer,char* context)
{
    //�޸����ݿ�
    char Mobile[48]= {0};
    char Phone[48]= {0};
    char Mail[48]= {0};
    char Mood[384]= {0};
    //char* data=UrlDecode(context);
    //sscanf(data,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    //free(data);
	sscanf(context,"%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]%*[^=]=%[^|]",Phone,Mobile,Mail,Mood);
    if(update_org_info(loginer,Phone,Mobile,Mail,Mood)<0)
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
}*/

char* Check_Photo(char* context)
{
	//val����pid=xxxxxxxx|md5=yyyyyyyyyy
	char pid[36]={0};
	char md5v[36]={0};
	if(sscanf(context,"%*[^=]=%[^|]%*[^=]=%s",pid,md5v)!=2)
		return "FAULT";
	int ret=web_check_avatar(pid,md5v);
	if(ret==1)
		return "OK";
	else
		return "FAULT";
}

/*char* Check_Photo(char* uid,char* val)
{
	int ret=check_user_photo(uid,md5);
	if(ret==0)
		return "OK";
	else
		return "FAULT";
}*/

/*char* Get_CIMS_ID(char* uid)
{
	char* ret=getCIMS_id(uid);
	if(ret!=NULL)
		return ret;
	else
		return "FAULT";
}*/

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
        // �ҵ���һ���滻��
        p = strstr(pi, pSrcRpl);

        if(p != NULL)
        {
            // ������һ���滻�����һ���滻���м���ַ���
            nLen = p - pi;
            memcpy(po, pi, nLen);
            // ������Ҫ�滻���ַ���
            memcpy(po + nLen, pDstRpl, nDstRplLen);
        }
        else
        {
            strcpy(po, pi);
            // ���û����Ҫ�������ַ���,˵��ѭ��Ӧ�ý���
            break;
        }
        pi = p + nSrcRplLen;
        po = po + nLen + nDstRplLen;
    }
    while (p != NULL);
}

int Count(char *const a,char *const b)
{
    //aΪ������bΪ�Ӵ�
    char *p=a,*q=b;
    int count=0;
    while(*p)
    {
        if ((*p==*q)&&(*q)) //ƥ������
        {
            p++;
            q++;
        }
        else //��ƥ��qҪ���ã����������м�������
        {
            p++;
            q=b;
        }
        if (!(*q)) //ƥ���qҪ����
        {
            count++;
            q=b;
        }
    }
    return count;
}*/

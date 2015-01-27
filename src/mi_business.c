#include "head.h"

void OnMiddleLogin(unsigned char result)
{
    if(RESPONSE==result)
    {
        sem_post(mi_send_recv_ctrl);
    }
}

void OnMiddleSchema(unsigned char result)
{
    if(RESPONSE==result)
    {
        sem_post(mi_send_recv_ctrl);
    }
}

static int check_tcp=0;	//获取组织结构标识位

void OnGetOnlineList(unsigned char Result, char* srcdata, int srclen)
{
    uint len=srclen-2*sizeof(uint32_t);
    char* rx=NULL;
    if(len>0)
    {
        rx=(char*)malloc(len);
        memcpy(rx,srcdata+2*sizeof(uint32_t),len);
    }
    else
        rx=NULL_OL();

    if(NULL!=PC_OL)
        free(PC_OL);
    PC_OL=rx;

	check_tcp=0;

	/*不再立刻通知客户端10-10
    UL p=user->next;
    while(p)
    {
        if(p->fd!=-1)
            if(p->flag==1)
            {
                Zero_RE(p->fd,"42",2,1);
                p->flag=0;
            }
            else
                insert_imf(p->il,"42",2);
        p=p->next;
    }
	*/
}

int MSG_RECV(unsigned char Result,char* srcdata,int srclen,int type)
{
    int ret=0;
    char* context=NULL;
    RMS64_CHARINFO ms64=NULL;
    RMS_GROUP mg=NULL;

    UL ul=NULL;
    switch(type)
    {
        case CTRLPERSON:
            ms64=(RMS64_CHARINFO)malloc(srclen);
            memcpy(ms64,srcdata,srclen);
            context=search_info(ms64->desid,CTRLPERSON);
            ul=get_point(user,ms64->desid);
            if(NULL!=ul)
            {
                if(ul->fd!=-1&&ul->flag==1)
                {
                    ret=Zero_RE(ul->fd,context,(strlen(context)),1);
                    ul->flag=0;
                }
            }
            if(context!=NULL&&strcmp("FALUT",context)!=0)
                free(context);
            free(ms64);
            break;
        case CTRLGROUP:
            mg=(RMS_GROUP)malloc(srclen);
            memcpy(mg,srcdata,srclen);
            //每个用户都发
            ret=get_group_mutil_user(mg->uid,mg->gid,CTRLGROUP);
            free(mg);
            break;
        case CTRLMUTIL:
            mg=(RMS_GROUP)malloc(srclen);
            memcpy(mg,srcdata,srclen);
            //每个用户都发
            ret=get_group_mutil_user(mg->uid,mg->gid,CTRLMUTIL);
            free(mg);
            break;
    }
    return ret;
}

void MSG_INFO(unsigned char Result,int type)
{
    uint len;
    char *context=NULL;
    switch(type)
    {
        case CTRLPERSON:
            context="update 5";
            break;
        case CTRLGROUP:
            context="update 8";
            break;
        case CTRLMUTIL:
            context="update 12";
            break;
    }
    UL p=user->next;
    while(p)
    {
        if(p->fd!=-1)
        {
			len=strlen(context);
            if(p->flag==1)
            {
                Zero_RE(p->fd,context,len,1);
                p->flag=0;
            }
            else
                insert_imf(p->il,context,len);
        }
        p=p->next;
    }
}

int Link_Mi()
{
    MSEXTAPPREG_REQ reg_req={0};
    reg_req.bp=BsnsPacket_init(MC_EXTAPP_CONN, REQUEST, NONE,sizeof(uint32_t));
    reg_req.ntype=EAT_BTANDRIOD;
    return MI_Write((char*)&reg_req,sizeof(MSEXTAPPREG_REQ),0);
}

int Send_MO_OL()
{
    int ret=0;
    flush_list(user);
    uint len=strlen(MO_OL)+1;
    MSEXTAPP_REG_REQ sendol=(MSEXTAPP_REG_REQ)malloc(sizeof(MSEXTAPPREG_REQ)+len);
    memset(sendol,0,sizeof(MSEXTAPP_REG_REQ)+len);
    memcpy(sendol->context,MO_OL,len-1);
    sendol->bp=BsnsPacket_init(MC_EXTAPP_SCHEMA, REQUEST,NONE, sizeof(uint32_t)+len-1);
    sendol->ntype=EAT_BTANDRIOD;
    ret=MI_Write((char*)sendol,sizeof(MSEXTAPPREG_REQ)+len-1,0);
    free(sendol);
    return ret;
}

int SEND_GET_PC_ONLINE_LIST()
{
    MSEXTAPPREG_REQ mc= {0};
    mc.bp=BsnsPacket_init(MC_EXTAPP_GETONLINELIST, REQUEST, NONE,sizeof(uint32_t));
    mc.ntype=EAT_BTPCINSTANT;
	int ret=MI_Write((char*)&mc,sizeof(MSEXTAPPREG_REQ),0);
	if(ret!=-1)
		check_tcp=1;
	return ret;
}

void* CHECK_MI_LINK(void* arg)
{
	pthread_detach(pthread_self()); //分离线程
	int time=0;
	for(;;)
	{
		sleep(240);
		SEND_GET_PC_ONLINE_LIST();
		sleep(3);
		if(check_tcp==1)
		{
			time++;
			if(time%2==0)
			{
				Exit_Mi_TCP();
				sleep(3);
				Init_Mi_TCP(MI_ADDR,MI_PORT);
			}
		}
	}
}

/*char Char2Int(char ch)
{
    if(ch>='0' && ch<='9')return (char)(ch-'0');
    if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
    if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
    return -1;
}

char Str2Bin(char *str)
{
    char tempWord[2];
    char chn;

    tempWord[0] = Char2Int(str[0]);                                //make the B to 11 -- 00001011
    tempWord[1] = Char2Int(str[1]);                                //make the 0 to 0  -- 00000000

    chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000

    return chn;
}

unsigned char* UrlDecode(char* str)
{
    char tmp[2];
    int i=0,idx=0,ndx;
    int len=strlen(str);
    unsigned char* out=(unsigned char*)malloc(len+1);
    memset(out,0,len+1);
    unsigned char* output=out;

    while(i<len)
    {
        if(str[i]=='%')
        {
            tmp[0]=str[i+1];
            tmp[1]=str[i+2];
            *(out++)=Str2Bin(tmp);
            i=i+3;
        }
        else if(str[i]=='+')
        {
            *(out++)=' ';
            i++;
        }
        else
        {
            *(out++)=str[i];
            i++;
        }
    }
    return output;
}*/

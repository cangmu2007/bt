#include "head.h"
#include "DESCryptor.h"

int check_up(char *uid,char *pass)
{
	char SQL[256]= {0};
	int len=strlen(pass);
	int temp;
	uint tlen;
	if((temp=len%8)>0)
		tlen=len+(8-temp);
	unsigned char deskey[24] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+' };
	unsigned char* des=(unsigned char*)malloc(tlen);
	memset(des,0,tlen);
	FinalD3desEncryption(deskey,pass , des , len);
	sprintf(SQL,"select id from customer_info where UserID='%s' and UserPassword='",uid);
	int i;
	uint slen=strlen(SQL);
	for(i=0; i<tlen; i++)
	{
		sprintf(SQL+slen+(i*2),"%.2x",*(des+i));
	}
	strcat(SQL,"\'");
	free(des);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
/*****************************************************************************************/

	int UID=0;
	DBINT res;
	if((res=dbresults(dbprocess))!=NO_MORE_RESULTS)
	{
		if(res==SUCCEED)
		{
			dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&UID);
			if(dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				if(UID>0)
				{
					tdbh->ctrling=0;
					return 1;
				}
			}
		}
	}
	tdbh->ctrling=0;
	return -1;
}

int set_group(char* uid,int gid,int type)
{
	char SQL[256]= {0};
	sprintf(SQL,"update group_customer_relation set NotifyType=%d where GroupId=%d and UserId='%s'",type,gid,uid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
/*****************************************************************************************/ 
	int ret=CTRLDB(tdbh,SQL);
	tdbh->ctrling=0;
	return ret;
}

int insert_group(char* uid,char* name,char* theme,char* id,int* gid,int type)
{
	char SQL[256]= {0};
	if(type==CTRLGROUP)
		sprintf(SQL,"insert into group_info(Name,Notify) values('%s','%s')",name,theme);
	else
		sprintf(SQL,"insert into mutil_info(Name,MutilBuilder) values('%s','%s')",theme,uid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;

	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
/*****************************************************************************************/

	if(type==CTRLGROUP)
		sprintf(SQL,"select IDENT_CURRENT('group_info') as GroupId");
	else
		sprintf(SQL,"select IDENT_CURRENT('mutil_info') as MutilId");

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}

	int TID=0;
	if(dbresults(dbprocess)!=SUCCEED)
	{
		tdbh->ctrling=0;
		return -1;
	}
	dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&TID);
	if (dbnextrow(dbprocess) == NO_MORE_ROWS)
	{
		tdbh->ctrling=0;
		return -1;
	}
	*gid=TID;

	char tmp[32]= {0};
	uint len=0;
	while(sscanf(id+len,"%[^|]",tmp)==1)
	{
		if(0!=strcmp(tmp,uid))
		{
			if(type==CTRLGROUP)
				sprintf(SQL,"insert into group_customer_relation(GroupID,UserId,Auth) values(%d,'%s',0)",TID,tmp);
			else
				sprintf(SQL,"insert into mutil_customer_relation(MutilID,UserId) values(%d,'%s')",TID,tmp);
			if(-1==CTRLDB(tdbh,SQL))
			{
				tdbh->ctrling=0;
				return -1;
			}
			memset(SQL,0,256);
		}
		len+=strlen(tmp)+1;
		memset(tmp,0,32);
	}
	if(type==CTRLGROUP)
		sprintf(SQL,"insert into group_customer_relation(GroupID,UserId,Auth) values(%d,'%s',2)",TID,uid);
	else
		sprintf(SQL,"insert into mutil_customer_relation(MutilID,UserId) values(%d,'%s')",TID,uid);
	
	int ret=CTRLDB(tdbh,SQL);
	tdbh->ctrling=0;
	return ret;
}

int insert_mutil_orger(int gid,char* ids)
{
	char SQL[128]= {0};
	char id[32]= {0};
	DbprocHandler tdbh=NULL;
	while(sscanf(ids,"%[^|]",id))
	{
		sprintf(SQL,"insert into mutil_customer_relation(MutilId,UserId) values(%d,'%s')",gid,id);

/*****************************************新增连接池**************************************/
	tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;

	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
/*****************************************************************************************/

		ids+=strlen(id)+1;
		memset(id,0,32);
		memset(SQL,0,128);
	}
	tdbh->ctrling=0;
	return 0;
}

int exit_group_mutil(char* uid,int gid,int type)
{
	char SQL[128]= {0};
	DBINT ret;

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	if(type==CTRLGROUP)
	{
		sprintf(SQL,"select Auth from group_customer_relation where GroupId=%d and UserId='%s'",gid,uid);
	dbcancel(dbprocess);
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}

		if(dbresults(dbprocess)!= SUCCEED)
		{
			tdbh->ctrling=0;
			return -1;
		}
		int auth;
		dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&auth);
		if(dbnextrow(dbprocess) == NO_MORE_ROWS)
		{
			tdbh->ctrling=0;
			return -1;
		}
		memset(SQL,0,128);
		if(auth==2)
		{
			sprintf(SQL,"delete from group_info where GroupId=%d",gid);

			if(-1==CTRLDB(tdbh,SQL))
			{
				tdbh->ctrling=0;
				return -1;
			}

			memset(SQL,0,128);
			sprintf(SQL,"delete from group_ctm_msg_mt where GroupId=%d",gid);
			
			if(-1==CTRLDB(tdbh,SQL))
			{
				tdbh->ctrling=0;
				return -1;
			}
			
			memset(SQL,0,128);
			sprintf(SQL,"select UserId from group_customer_relation where GroupId=%d",gid);
			
			dbcancel(dbprocess);
			if(-1==CTRLDB(tdbh,SQL))
			{
				tdbh->ctrling=0;
				return -1;
			}

			char tmp[32]= {0};
			while((ret=dbresults(dbprocess))!= NO_MORE_RESULTS)
			{
				if(ret==SUCCEED)
				{
					dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)tmp);
					while (dbnextrow(dbprocess) != NO_MORE_ROWS)
					{
						UL usr=get_point(user,tmp);
						if(NULL!=usr)
						{
							if(usr->fd!=-1)
							{
								if(usr->flag==1)
								{
									Zero_RE(usr->fd,"8",1,1);
									usr->flag=0;
								}
								else
									insert_imf(usr->il,"8",1);
							}
						}
						memset(tmp,0,32);
					}
				}
			}
			memset(SQL,0,128);
			sprintf(SQL,"delete from group_customer_relation where GroupId=%d",gid);
		}
		else
		{
			sprintf(SQL,"delete from group_customer_relation where GroupId=%d and UserId='%s'",gid,uid);
		}
		if(-1==CTRLDB(tdbh,SQL))
		{
			tdbh->ctrling=0;
			return -1;
		}
	}
	else
	{
		sprintf(SQL,"delete from mutil_customer_relation where GroupId=%d and UserId='%s'",gid,uid);
		if(-1==CTRLDB(tdbh,SQL))
		{
			tdbh->ctrling=0;
			return -1;
		}
	}
	tdbh->ctrling=0;
	return 0;
}

int update_org_info(char* uid,char* Phone,char* Mobile,char* Mail,char* Mood)
{
	char SQL[512]= {0};
	strcpy(SQL,"update customer_info set ");
	char tmp[512]= {0};
	if(NULL!=Phone)
	{
		sprintf(tmp,"Phone='%s'",Phone);
		strcat(SQL,tmp);
		memset(tmp,0,512);
	}
	if(NULL!=Mobile)
	{
		if(NULL!=Phone)
			strcat(SQL,",");
		sprintf(tmp,"Mobile='%s'",Mobile);
		strcat(SQL,tmp);
		memset(tmp,0,512);
	}
	if(NULL!=Mail)
	{
		if(NULL!=Phone||NULL!=Mobile)
			strcat(SQL,",");
		sprintf(tmp,"Mail='%s'",Mail);
		strcat(SQL,tmp);
		memset(tmp,0,512);
	}
	if(NULL!=Mood)
	{
		if(NULL!=Phone||NULL!=Mobile||NULL!=Mail)
			strcat(SQL,",");
		sprintf(tmp,"Mood='%s'",Mood);
		strcat(SQL,tmp);
		memset(tmp,0,512);
	}
	sprintf(tmp," where UserId='%s'",uid);
	strcat(SQL,tmp);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
/*****************************************************************************************/
	tdbh->ctrling=0;
	return 0;
}

int delete_msg(char* ids,int type)
{
	char SQL[1024]= {0};
	char id[10]= {0};
	uint len=0;
	int ret=-1;
	/*while((sscanf(ids+len,"%[^|]",id)==1))
	{
		switch(type)
		{
			case CTRLPERSON:
				sprintf(SQL,"delete from ctm_ctm_msg_mt where id=%s",id);
				break;
			case CTRLGROUP:
				sprintf(SQL,"delete from group_ctm_msg_mt where id=%s",id);
				break;
			case CTRLMUTIL:
				sprintf(SQL,"delete from multi_ctm_msg_mt where id=%s",id);
				break;
		}
		CTRLDB(SQL);
		memset(SQL,0,1024);
		len+=strlen(id)+1;
		memset(id,0,8);
	}*/
	
	char tmp[512]={0};
	char temp[12]={0};
	while((sscanf(ids+len,"%[^|]",id)==1))
	{
		sprintf(temp,"%s,",id);
		strcat(tmp,temp);
		memset(temp,0,12);		
		len+=strlen(id)+1;
		memset(id,0,10);
	}
	uint flg=strlen(tmp);
	tmp[flg-1]='\0';
	switch(type)
	{
		case CTRLPERSON:
			sprintf(SQL,"delete from ctm_ctm_msg_mt where id in (%s)",tmp);
			break;
		case CTRLGROUP:
			sprintf(SQL,"delete from group_ctm_msg_mt where id in (%s)",tmp);
			break;
		case CTRLMUTIL:
			sprintf(SQL,"delete from multi_ctm_msg_mt where id in (%s)",tmp);
			break;
	}

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		ret=-1;
	else
	{
		ret=CTRLDB(tdbh,SQL);
		tdbh->ctrling=0;
	}
/*****************************************************************************************/

	if(ret==-1)
		printf("delete messages error:%s!",ids);
	return ret;
}

int insert_talklist(char* src,char* des,char* context,uint32_t len,int type)
{
	unsigned char SQL[256]= {0};
	char userid[32]= {0};
	int dbflag=0,flag=0,i,j,temp,ret=0,num=0;
	unsigned char *d=NULL;
	char* ndata=NULL;
	unsigned char deskey[24] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+' };
	unsigned char* cdes=NULL;

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
	//tdbh->ctrling=1;
	if(NULL!=strstr(context,"<img src"))
	{
		/*char extt[16]= {0};
		memcpy(extt,context+len-6,3);
		sprintf(SQL,"insert into andriod_image(ImageLen,ImageExt,SrcUserId,UnitId,LinkType,ImageData) values(%u,'.%s','%s','%s',%d,0x",len-23,extt,src,des,type-1);
		dbfreebuf(dbprocess);
		dbcmd(dbprocess,SQL);
		for(i=0; i<len-23; i++)
		{
			dbfcmd(dbprocess,"%.2x",(*((unsigned char *)(context+10+i))));
		}
		dbcmd(dbprocess,")");*/

/******************************************图片存储加密10-13*********************************/
		uint templen=0;
		uint imagelen=len-23;
		if((templen=imagelen%8)>0)
			imagelen=imagelen+(8-templen);
		unsigned char* imagedes=(unsigned char*)malloc(imagelen);
		memset(imagedes,0,imagelen);
		FinalD3desEncryption(deskey,context+10, imagedes, len-23);

		char extt[16]= {0};
		memcpy(extt,context+len-6,3);
		sprintf(SQL,"insert into andriod_image(ImageLen,ImageExt,SrcUserId,UnitId,LinkType,DataType,ImageData) values(%u,'.%s','%s','%s',%d,1,0x",imagelen+16,extt,src,des,type-1);
		dbfreebuf(dbprocess);
		dbcmd(dbprocess,SQL);
		char imglen[8]={0};
		char desimglen[8]={0};
		sprintf(imglen,"%d",len-23);
		sprintf(desimglen,"%d",imagelen);
		
		//明文长度
		for(i=0;i<8;i++)
			dbfcmd(dbprocess,"%.2x",*(imglen+i));
		//密文长度
		for(i=0;i<8;i++)
			dbfcmd(dbprocess,"%.2x",*(desimglen+i));
		//密文写入缓冲区
		for(i=0;i<imagelen;i++)
			dbfcmd(dbprocess,"%.2x",(*((unsigned char *)(imagedes+i))));
		free(imagedes);
		imagedes=NULL;

		/*for(i=0; i<len-23; i++)
		{
			dbfcmd(dbprocess,"%.2x",(*((unsigned char *)(context+10+i))));
		}*/
		dbcmd(dbprocess,")");
/********************************************************************************************/

		dbflag=0;
		if(dbsqlexec(dbprocess) == FAIL)
		{
			printf("dbsqlexec error!\n");
			if(ReConnect(tdbh)<0)
			{
				tdbh->flg=0;
				if(linkcount>0)
					linkcount--;
			}
			tdbh->ctrling=0;
			dbflag=1;
			return -1;
		}
		dbcancel(dbprocess);     //清除上次查询结果
		int pid;
		char ext[16]= {0};
		memset(SQL,0,256);
		sprintf(SQL,"select top 1 id,ImageExt from andriod_image order by id desc");
		dbfreebuf(dbprocess);
		dbcmd(dbprocess,SQL);
		dbflag=0;
		if(dbsqlexec(dbprocess) == FAIL)
		{
			printf("dbsqlexec error!\n");
			if(ReConnect(tdbh)<0)
			{
				tdbh->flg=0;
				if(linkcount>0)
					linkcount--;
			}
			tdbh->ctrling=0;
			dbflag=1;
			return -1;
		}
		if(dbresults(dbprocess) == SUCCEED)
		{
			dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&pid);
			dbbind(dbprocess, 2, CHARBIND, (DBINT)0, (BYTE*)ext);
			if(dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				ndata=(char*)malloc(48);
				memset(ndata,0,48);
				sprintf(ndata,"<img src=\"%d%s\"/>",pid,ext);
				memset(SQL,0,256);
				flag=1;
			}
		}
	}

	if(type==CTRLPERSON)
	{
		if(1==flag)
			d=ndata;
		else
			d=UrlDecode(context);
		uint clen=strlen(d);
		uint blen=clen;
		if((temp=clen%8)>0)
			clen=clen+(8-temp);
		unsigned char* cdes=(unsigned char*)malloc(clen);
		memset(cdes,0,clen);
		FinalD3desEncryption(deskey,d , cdes , blen);
		sprintf(SQL,"insert into ctm_ctm_msg_mt(DstUserId,SrcUserId,DataLen,Sendtime,Data) values('%s','%s',%u,getdate(),0x",des,src,clen,cdes);
		dbcmd(dbprocess,SQL);
		for(i=0; i<clen; i++)
			dbfcmd(dbprocess,"%.2x",*(cdes+i));
		dbcmd(dbprocess,")");
		dbflag=0;   //防止过分频繁操作数据库
		dbcancel(dbprocess);
		if(dbsqlexec(dbprocess) == FAIL)
		{
			printf("dbsqlexec error!\n");
			if(ReConnect(tdbh)<0)
			{
				tdbh->flg=0;
				if(linkcount>0)
					linkcount--;
			}
			dbflag=1;
			tdbh->ctrling=0;
			return -1;
		}
		if(flag==1)
		{
			uint tlen=strlen(d);
			MS64_CHARINFO ms64=(MS64_CHARINFO)malloc(sizeof(MS64CHARINFO)+tlen);
			memset(ms64,0,sizeof(MS64CHARINFO)+tlen);
			strcpy(ms64->srcid,src);
			strcpy(ms64->desid,des);
			memcpy(ms64->context,d,tlen);
			ms64->bp=BsnsPacket_init(MC_BTANDRIOD_PTOP_MSG, REQUEST, NONE,64+tlen);
			ret=MI_Write((char*)ms64,sizeof(MS64CHARINFO)+tlen,1);
			num=MSG_RECV(RESPONSE,(char*)ms64->srcid,64,type);
			free(ms64);
		}
		free(d);
		if(ret==-1||num==-1)
		{
			tdbh->ctrling=0;
			return -2;
		}
	}
	else
	{
		if(type==CTRLGROUP)
			sprintf(SQL,"select UserId from group_customer_relation where GroupID=%d and UserId<>'%s'",atoi(des),src);
		if(type==CTRLMUTIL)
			sprintf(SQL,"select UserId from mutil_customer_relation where MutilID=%d and UserId<>'%s'",atoi(des),src);
		dbcancel(dbprocess);
		if(-1==CTRLDB(tdbh,SQL))
		{
			tdbh->ctrling=0;
			return -1;
		}
		char* guser=NULL;
		DBINT res;
		uint t=0;
		while((res=dbresults(dbprocess)) != NO_MORE_RESULTS)
		{
			if(res==SUCCEED)
			{
				dbbind(dbprocess, 1, CHARBIND, (DBINT)0, (BYTE*)userid);
				guser=(char*)malloc(32);
				while (dbnextrow(dbprocess) != NO_MORE_ROWS)
				{
					if(t>0)
						guser=(char*)realloc(guser,32*(t+1));
					memset(guser+32*t,0,32);
					strcpy(guser+32*t,userid);
					memset(userid,0,32);
					t++;
				}
			}
		}

		if(1==flag)
			d=ndata;
		else
			d=UrlDecode(context);
		uint tlen=strlen(d);
		uint blen=tlen;
		if((temp=tlen%8)>0)
			tlen=tlen+(8-temp);
		unsigned char* cdes=(unsigned char*)malloc(tlen);
		memset(cdes,0,tlen);
		FinalD3desEncryption(deskey,d , cdes , blen);
		for(j=0; j<t; j++)
		{
			dbfreebuf(dbprocess);
			if(type==CTRLGROUP)
				sprintf(SQL,"insert into group_ctm_msg_mt(GroupID,UserId,SrcUserId,DataLen,Sendtime,Data) values(%d,'%s','%s',%u,getdate(),0x",atoi(des),guser+(j*32),src,tlen);
			if(type==CTRLMUTIL)
				sprintf(SQL,"insert into multi_ctm_msg_mt(MultiID,UserId,SrcUserId,DataLen,Sendtime,Data) values(%d,'%s','%s',%u,getdate(),0x",atoi(des),guser+(j*32),src,tlen);
			dbcmd(dbprocess,SQL);
			for(i=0; i<tlen; i++)
				dbfcmd(dbprocess,"%.2x",*(cdes+i));
			dbcmd(dbprocess,")");
			dbflag=0;   //防止过分频繁操作数据库
			if(dbsqlexec(dbprocess) == FAIL)
			{
				printf("dbsqlexec error!\n");
			if(ReConnect(tdbh)<0)
			{

				tdbh->flg=0;
				if(linkcount>0)
				linkcount--;
			}
			dbflag=1;
				tdbh->ctrling=0;
				return -1;
			}
		}
		free(guser);
		if(flag==1)
		{
			uint tlen=strlen(d);
			MS_GROUP mg= (MS_GROUP)malloc(sizeof(MSGROUP)+tlen);
			memset(mg,0,sizeof(MSGROUP)+tlen);
			strcpy(mg->uid,src);
			mg->gid=atoi(des);
			memcpy(mg->context,d,tlen);
			mg->bp=BsnsPacket_init((type==2)?MC_BTANDRIOD_GROUP_MSG:MC_BTANDRIOD_MULTI_MSG, REQUEST, NONE,32+sizeof(uint32_t)+tlen);
			ret=MI_Write((char*)mg,sizeof(MSGROUP)+tlen,1);
			num=MSG_RECV(RESPONSE,(char*)mg->uid,32+sizeof(uint32_t),type);
			free(mg);
		}
		free(d);
		if(ret==-1||num==-1)
		{
			tdbh->ctrling=0;
			return -2;
		}
	}
	tdbh->ctrling=0;	//解除使用标记
	if(cdes!=NULL&&flag==1)
		free(cdes);
	return flag==1?1:0;
}

int check_user_group_or_mutil(int gid,char* uid,int type)
{
	char SQL[128]= {0};
	if(type==CTRLGROUP)
		sprintf(SQL,"select UserId from group_customer_relation where UserId='%s' and GroupId=%d",uid,gid);
	else
		sprintf(SQL,"select UserId from mutil_customer_relation where UserId='%s' and MutilId=%d",uid,gid);
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
	int ret=dbresults(dbprocess);
	tdbh->ctrling=0;
	return ret;
}

char* get_group_or_mutil(char* id,int type)
{
	char SQL[256]= {0};
	if(type==CTRLGROUP)
		sprintf(SQL,"select group_info.GroupId,Name,NotifyType,Notify from group_customer_relation,group_info where group_customer_relation.Userid='%s' and group_customer_relation.GroupID=group_info.GroupId",id);
	else
		sprintf(SQL,"select mutil_info.MutilId,Name from mutil_customer_relation,mutil_info where mutil_customer_relation.UserId='%s' and mutil_customer_relation.MutilId=mutil_info.MutilId",id);
	
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	char* results=(char*)malloc(52);
	memset(results,0,52);
	strcpy(results,MO_XML_HEAD);
	strcat(results,"<conpany>");
	DBINT result;
	int GID;
	char Notify[1024]= {0};
	uint len=0,tlen=0;
	char tmp[1536]= {0};
	char Name[48]= {0};
	int Auth=0;
	while((result=dbresults(dbprocess))!=NO_MORE_RESULTS)
	{
		if(result==SUCCEED)
		{
			dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&GID);
			if(type==CTRLGROUP)
			{
				dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)Name);
				dbbind(dbprocess, 3, INTBIND, (DBINT)0, (BYTE*)&Auth);
				dbbind(dbprocess, 4, CHARBIND, (DBCHAR)0, (BYTE*)Notify);
				while (dbnextrow(dbprocess) != NO_MORE_ROWS)
				{
					//if(Notify==NULL)
						//strcpy(Notify,"null");
					sprintf(tmp,"<group><name><![CDATA[%s]]></name><signture><![CDATA[%s]]></signture><isshiled>%d</isshiled><id>%d</id></group>",Name,Notify,Auth,GID);
					memset(Notify,0,1024);
					memset(Name,0,48);
					len=strlen(results);
					tlen=strlen(tmp);
					results=(char*)realloc(results,(len+1+tlen));
					memset(results+len,0,tlen+1);
					strcat(results,tmp);
					memset(tmp,0,1536);
				}
			}
			else if(type==CTRLMUTIL)
			{
				dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)Notify);
				while (dbnextrow(dbprocess) != NO_MORE_ROWS)
				{
<<<<<<< HEAD
					//if(Notify==NULL)
						//strcpy(Notify,"null");
=======
					if(Notify==NULL)
						strcpy(Notify,"null");
>>>>>>> origin/master
					sprintf(tmp,"<group><name><![CDATA[%s]]></name><id>%d</id></group>",Notify,GID);
					memset(Notify,0,1024);
					len=strlen(results);
					tlen=strlen(tmp);
					results=(char*)realloc(results,(len+1+tlen));
					memset(results+len,0,tlen+1);
					strcat(results,tmp);
					memset(tmp,0,1536);
				}
			}
		}
	}
	tdbh->ctrling=0;
	len=strlen(results);
	results=(char*)realloc(results,len+11);
	memset(results+len,0,11);
	strcat(results,"</conpany>");
	return results;
}

char* get_member(int gid,int type)
{
	char SQL[128]= {0};
	if(type==CTRLGROUP)
		sprintf(SQL,"select UserID from group_customer_relation where GroupID=%d",gid);
	else
		sprintf(SQL,"select UserID from mutil_customer_relation where MutilId=%d",gid);
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	char *xml=(char*)malloc(52);
	memset(xml,0,52);
	strcpy(xml,MO_XML_HEAD);
	strcat(xml,"<conpany>");
	char UID[32]= {0};
	char tmp[64]= {0};
	DBINT result;
	uint len,tlen;
	while((result=dbresults(dbprocess)) != NO_MORE_RESULTS)
	{
		if(result==SUCCEED)
		{
			dbbind(dbprocess, 1, CHARBIND, (DBINT)0, (BYTE*)UID);
			while (dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				sprintf(tmp,"<member><id>%s</id></member>",UID);
				memset(UID,0,32);
				len=strlen(xml);
				tlen=strlen(tmp);
				xml=(char*)realloc(xml,len+1+tlen);				
				memset(xml+len,0,tlen+1);
				strcat(xml,tmp);
				memset(tmp,0,64);
			}
		}
	}
	tdbh->ctrling=0;
	len=strlen(xml);
	xml=(char*)realloc(xml,len+11);
	memset(xml+len,0,11);
	strcat(xml,"</conpany>");
	return xml;
}

char* get_info(char* uid)
{
	char* xml=(char*)malloc(256);
	memset(xml,0,256);
	strcpy(xml,MO_XML_HEAD);
	strcat(xml,"<conpany>");
	uint len=strlen(xml);
	char SQL[128]= {0};
	char Phone[32]= {0};
	char Mobile[32]= {0};
	char Mail[32]= {0};
	sprintf(SQL,"select Mobile,Phone,Mail from customer_info where UserId='%s'",uid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
	{
		free(xml);
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		free(xml);
		tdbh->ctrling=0;
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	if(dbresults(dbprocess)!=NO_MORE_RESULTS)
	{
		dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)Mobile);
		dbbind(dbprocess,2,CHARBIND,(DBINT)0,(BYTE*)Phone);
		dbbind(dbprocess,3,CHARBIND,(DBINT)0,(BYTE*)Mail);
		if(dbnextrow(dbprocess) != NO_MORE_ROWS)
		{
			if(Mobile==0)
				stacpy(Mobile,"null");
			if(Phone==0)
				stacpy(Phone,"null");
			if(Mail==0)
				stacpy(Mail,"null");
			sprintf(xml+len,"<workphone>%s</workphone><mobilephone>%s</mobilephone><email>%s</email></conpany>",Phone,Mobile,Mail);
			tdbh->ctrling=0;
			return xml;
		}
	}
	tdbh->ctrling=0;
	free(xml);
<<<<<<< HEAD
	return NULL;
=======
	return "FAULT";
>>>>>>> origin/master
}

char* get_photo(char* uid)
{
	char SQL[128]= {0};
	uint size;
	sprintf(SQL,"select PhotoSize from customer_info where UserId='%s'",uid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	if(dbresults(dbprocess)==SUCCEED)
	{
		dbbind(dbprocess,1,INTBIND,(DBINT)0,(BYTE*)&size);
		if(dbnextrow(dbprocess) != NO_MORE_ROWS)
		{
			if(size==0)
			{
				tdbh->ctrling=0;
<<<<<<< HEAD
				return NULL;
=======
				return "FAULT";
>>>>>>> origin/master
			}
			memset(SQL,0,128);
			sprintf(SQL,"select Photo from customer_info where UserId='%s'",uid);
			dbcancel(dbprocess);    //清除上次查询结果
			if(-1!=CTRLDB(tdbh,SQL))
			{
				if(dbresults(dbprocess)==SUCCEED)
				{
					uint8_t *photo=(uint8_t*)malloc(size+10);
					memset(photo,0,size+10);
					sprintf(photo,"%u",size);
					dbbind(dbprocess,1,VARYBINBIND,(DBINT)0,(BYTE*)(photo+8));
					if(dbnextrow(dbprocess) != NO_MORE_ROWS)
					{
						tdbh->ctrling=0;
						return photo;
					}
					free(photo);
				}
			}
		}
	}
	tdbh->ctrling=0;
<<<<<<< HEAD
	return NULL;
=======
	return "FAULT";
>>>>>>> origin/master
}

char* get_picture(int pid,char* ext)
{
	unsigned char deskey[24] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+' };
	char SQL[128]= {0};
	sprintf(SQL,"select ImageLen,DataType from andriod_image where id=%d and ImageExt='.%s'",pid,ext);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
<<<<<<< HEAD
		return NULL;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
=======
		return "FAULT";
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

>>>>>>> origin/master
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	}
	if(dbresults(dbprocess)==SUCCEED)
	{
		int size,datatype;
		dbbind(dbprocess,1,INTBIND,(DBINT)0,(BYTE*)&size);
		dbbind(dbprocess,2,INTBIND,(DBINT)0,(BYTE*)&datatype);
		if(dbnextrow(dbprocess) != NO_MORE_ROWS)
		{
			if((datatype!=1)||(size>MAX_DATA-24000))	//类型判断
			{
				tdbh->ctrling=0;
<<<<<<< HEAD
				return NULL;
=======
				return "FAULT";
>>>>>>> origin/master
			}
			memset(SQL,0,128);
			sprintf(SQL,"select ImageData from andriod_image where id=%d and ImageExt='.%s'" ,pid,ext);
			dbcancel(dbprocess);    //清除上次查询结果
			if(-1==CTRLDB(tdbh,SQL))
			{
				tdbh->ctrling=0;
<<<<<<< HEAD
				return NULL;
=======
				return "FAULT";
>>>>>>> origin/master
			}
			if(dbresults(dbprocess)==SUCCEED)
			{
/*************************************************图片解密10-13*****************************************/
				uint8_t *tmppic=(uint8_t*)malloc(size+2);
				if(tmppic==NULL)
				{
					tdbh->ctrling=0;
<<<<<<< HEAD
					return NULL;
=======
					return "FAULT";
>>>>>>> origin/master
				}
				dbbind(dbprocess,1,VARYBINBIND,(DBINT)0,(BYTE*)tmppic);
				if(dbnextrow(dbprocess)!=NO_MORE_ROWS)
				{
					//明文长度
					uint imglen=atoi(tmppic+2);
					//密文长度
					uint desimglen=atoi(tmppic+10);

<<<<<<< HEAD
					uint8_t *pic=(uint8_t*)malloc(desimglen+10);
=======
					uint8_t *pic=(uint8_t*)malloc(imglen+10);
>>>>>>> origin/master
					if(pic==NULL)
					{
						free(tmppic);
						tdbh->ctrling=0;
<<<<<<< HEAD
						return NULL;
					}
					memset(pic,0,desimglen+10);
=======
						return "FAULT";
					}
					memset(pic,0,imglen+10);
>>>>>>> origin/master
					strcpy(pic,tmppic+2);
					FinalD3desDecryption(deskey,tmppic+18,pic+10,desimglen);
					free(tmppic);
					tdbh->ctrling=0;
					return pic;
				}
				free(tmppic);
/*******************************************************************************************************/
				/*uint8_t *pic=(uint8_t*)malloc(size+10);
				memset(pic,0,size+10);
				sprintf(pic,"%u",size);
				dbbind(dbprocess,1,VARYBINBIND,(DBINT)0,(BYTE*)(pic+8));
				if(dbnextrow(dbprocess) != NO_MORE_ROWS)
				{
					return pic;
				}
				free(pic);*/
			}
		}
	}
	tdbh->ctrling=0;
<<<<<<< HEAD
	return NULL;
=======
	return "FAULT";
>>>>>>> origin/master
}
/******************************废弃*******************************/
int get_org_stu(int dpmid)
{
	char depart[1024]= {0};
	int ret=0;
	/*char SQL[128]={0};
	  sprintf(SQL,"select DpmName from department_info where DpmId=%d",dpmid);
	  dbcancel(dbprocess);
	  if(-1==CTRLDB(SQL))
	  return -1;
	  if(dbresults(dbprocess)==SUCCEED)
	  {
	  dbbind(dbprocess,1,CHARBIND,(DBCHAR)0,(BYTE*)depart);
	  if (dbnextrow(dbprocess) != NO_MORE_ROWS)
	  {
	  if(strcmp(depart,"")==0)
	  ret=-1;
	  goto _ret;
	  }
	  }*/
	strcpy(depart,"/");
	if(org_stu!=NULL&&0!=strcmp(org_stu,MO_XML_HEAD))
		free(org_stu);
	org_stu=(char*)malloc(52);
	memset(org_stu,0,52);
	strcpy(org_stu,MO_XML_HEAD);
	strcat(org_stu,"<conpany>");
	if(get_subdepartmemt_user(depart,dpmid)<0)
	{
		free(org_stu);
		org_stu=MO_XML_HEAD;
		free(tmp_stu);
		ret=-1;
		goto _ret;
	}
	org_stu=(char*)realloc(org_stu,60+strlen(tmp_stu));
	strcat(org_stu,tmp_stu);
	free(tmp_stu);
	tmp_stu=NULL;
	strcat(org_stu,"</conpany>");
_ret:
	return ret;
}

int get_subdepartmemt_user(char* departmemt,int did)
{
	int ret=0;
	char SQL[512]= {0};
	sprintf(SQL,"select customer_info.UserId,Name,Sex,Mood from customer_info,department_customer_relation where customer_info.UserId=department_customer_relation.UserId and DpmId=%d",did);
	
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/
	
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
	char tmp[512]= {0};
	char UserId[32]= {0};
	char Name[48]= {0};
	int Sex=0;
	char Mood[384]= {0};
	DBINT results;
	uint len=0,tlen=0;
	while((results=dbresults(dbprocess))!=NO_MORE_RESULTS)
	{
		if(results==SUCCEED)
		{
			dbbind(dbprocess,1,CHARBIND,(DBCHAR)0,(BYTE*)UserId);
			dbbind(dbprocess,2,CHARBIND,(DBCHAR)0,(BYTE*)Name);
			dbbind(dbprocess,3,INTBIND,(DBINT)0,(BYTE*)&Sex);
			dbbind(dbprocess,4,CHARBIND,(DBCHAR)0,(BYTE*)Mood);
			while (dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				if(Mood[0]=='\0')
					strcpy(Mood,"null");
				sprintf(tmp,"<worker><id>%s</id><name>%s</name><sex>%d</sex><signture>%s</signture><department><![CDATA[%s]]></department></worker>",UserId,Name,Sex,Mood,departmemt);
				memset(UserId,0,32);
				memset(Name,0,48);
				memset(Mood,0,384);
				tlen=strlen(tmp);
				if(tmp_stu==NULL)
				{
					tmp_stu=(char*)malloc(1+tlen);
					memset(tmp_stu,0,1+tlen);
				}
				else
				{
					len=strlen(tmp_stu);
					tmp_stu=(char*)realloc(tmp_stu,len+1+tlen);
					memset(tmp_stu+len,0,tlen+1);
				}
				strcpy(tmp_stu+len,tmp);
				memset(tmp,0,512);
			}
		}
	}

	memset(SQL,0,512);
	sprintf(SQL,"select DpmId,DpmName from department_info where DpmId in (select SubordinateId from department_department_relation where department_department_relation.PrincipalId=%d)",did);
	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
	char DpmName[192]= {0};
	int tdid=0;
	subdpmt sd=NULL,p=NULL,pre;
	while((results=dbresults(dbprocess))!=NO_MORE_RESULTS)
	{
		if(results==SUCCEED)
		{
			dbbind(dbprocess,1,INTBIND,(DBINT)0,(BYTE*)&tdid);
			dbbind(dbprocess,2,CHARBIND,(DBCHAR)0,(BYTE*)DpmName);
			while (dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				p=(subdpmt)malloc(sizeof(subdpm));
				memset(p,0,sizeof(subdpm));
				sprintf(p->dpm,"%s>%s",departmemt,DpmName);
				p->did=tdid;
				tdid=0;
				memset(DpmName,0,192);
				if(sd==NULL)
					sd=p;
				else
					pre->next=p;
				pre=p;
			}
			if(p!=NULL)
				p->next=NULL;
		}
	}
	subdpmt deele=sd,next;
	while(deele)
	{
		if(-1==get_subdepartmemt_user(deele->dpm,deele->did))
		{
			ret=-1;
		}
		next=deele->next;
		free(deele);
		deele = next;
	}
	tdbh->ctrling=0;
	return ret;
}
/******************************废弃*******************************/

char* search_info(char* loginer,int type)
{
	char SQL[512]= {0};
	char SrcUserId[32]= {0};
	char SendTime[20]= {0};
	char UserId[32]= {0};
<<<<<<< HEAD
	char Data[4100]= {0};
	char dData[4100]= {0};
=======
	char Data[4098]= {0};
	char dData[4096]= {0};
>>>>>>> origin/master
	int mid,Datalen;
	char tmp[4608]= {0};
	unsigned char deskey[24] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+' };
	DBINT result;
	uint len=0,tlen;
	int ret=0;
	char* xml=(char*)malloc(52);
	memset(xml,0,52);
	strcpy(xml,MO_XML_HEAD);
	strcat(xml,"<conpany>");

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
<<<<<<< HEAD
		return NULL;
=======
		return "FAULT";
>>>>>>> origin/master
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	if(len<5120&&(type==CTRLALL||type==CTRLPERSON))
	{
		sprintf(SQL,"select SrcUserId,Data,convert(nvarchar(20),SendTime,20) as SendTime,id,DataLen from ctm_ctm_msg_mt where SrcUserId<>'%s' and DstUserId='%s'",loginer,loginer);
		dbcancel(dbprocess);    //清除上次查询结果
		if(-1==CTRLDB(tdbh,SQL))
		{
			free(xml);
			tdbh->ctrling=0;
<<<<<<< HEAD
			return NULL;
=======
			return "FAULT";
>>>>>>> origin/master
		}
		while((result=dbresults(dbprocess))!= NO_MORE_RESULTS)
		{
			if(result==SUCCEED)
			{
				dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)SrcUserId);
				dbbind(dbprocess,2,VARYBINBIND,(DBINT)0,(BYTE*)Data);
				dbbind(dbprocess,3,CHARBIND,(DBINT)0,(BYTE*)SendTime);
				dbbind(dbprocess,4,INTBIND,(DBINT)0,(BYTE*)&mid);
				dbbind(dbprocess,5,INTBIND,(DBINT)0,(BYTE*)&Datalen);
				while(dbnextrow(dbprocess)!=NO_MORE_ROWS)
				{
					if(len<5120)
						FinalD3desDecryption(deskey,Data+2,dData,Datalen);
					sprintf(tmp,"<message><sender>%s</sender><type>1</type><id>%d</id><typeid>null</typeid><time>%s</time><context><![CDATA[%s]]></context></message>",SrcUserId,mid,SendTime,dData);
					len=strlen(xml);
					tlen=strlen(tmp);
					xml=(char*)realloc(xml,len+1+tlen);
					memset(xml+len,0,tlen+1);
					strcpy(xml+len,tmp);
					memset(tmp,0,tlen);
					memset(SrcUserId,0,32);
					memset(SendTime,0,20);
					memset(Data,0,4098);
					memset(dData,0,4096);
					ret=1;
				}
			}
		}
		memset(SQL,0,512);
	}

	if(len<5120&&(type==CTRLALL||type==CTRLGROUP))
	{
		sprintf(SQL,"select group_ctm_msg_mt.GroupId,SrcUserId,Data,convert(nvarchar(20),SendTime,20) as SendTime,group_ctm_msg_mt.id,DataLen from group_ctm_msg_mt,group_customer_relation where group_ctm_msg_mt.SrcUserId<>'%s' and group_ctm_msg_mt.UserId='%s' and group_ctm_msg_mt.UserId=group_customer_relation.UserId and group_ctm_msg_mt.GroupId=group_customer_relation.GroupId and group_customer_relation.NotifyType<>2",loginer,loginer);
		dbcancel(dbprocess);    //清除上次查询结果
		if(-1==CTRLDB(tdbh,SQL))
		{
			free(xml);
			tdbh->ctrling=0;
<<<<<<< HEAD
			return NULL;
=======
			return "FAULT";
>>>>>>> origin/master
		}
		while((result=dbresults(dbprocess))!= NO_MORE_RESULTS)
		{
			if(result==SUCCEED)
			{
				dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)SrcUserId);
				dbbind(dbprocess,2,CHARBIND,(DBINT)0,(BYTE*)UserId);
				dbbind(dbprocess,3,VARYBINBIND,(DBINT)0,(BYTE*)Data);
				dbbind(dbprocess,4,CHARBIND,(DBINT)0,(BYTE*)SendTime);
				dbbind(dbprocess,5,INTBIND,(DBINT)0,(BYTE*)&mid);
				dbbind(dbprocess,6,INTBIND,(DBINT)0,(BYTE*)&Datalen);
				while(dbnextrow(dbprocess)!=NO_MORE_ROWS)
				{
					if(len<5120)
					{
						FinalD3desDecryption(deskey,Data+2,dData,Datalen);
						sprintf(tmp,"<message><sender>%s</sender><type>2</type><id>%d</id><typeid>%s</typeid><time>%s</time><context><![CDATA[%s]]></context></message>",SrcUserId,mid,UserId,SendTime,dData);
						len=strlen(xml);
						tlen=strlen(tmp);
						xml=(char*)realloc(xml,len+1+tlen);
						memset(xml+len,0,tlen+1);
						strcpy(xml+len,tmp);
						memset(tmp,0,tlen);
						memset(SrcUserId,0,32);
						memset(SendTime,0,20);
						memset(Data,0,4098);
						memset(dData,0,4096);
						memset(UserId,0,32);
						ret=1;
					}
				}
			}
		}
		memset(SQL,0,512);
	}

	if(len<5120&&(type==CTRLALL||type==CTRLMUTIL))
	{
		sprintf(SQL,"select MultiId,SrcUserId,Data,convert(nvarchar(20),SendTime,20) as SendTime,id,DataLen from multi_ctm_msg_mt where SrcUserId<>'%s' and UserId='%s'",loginer,loginer);
		dbcancel(dbprocess);    //清除上次查询结果
		if(-1==CTRLDB(tdbh,SQL))
		{
			free(xml);
			tdbh->ctrling=0;
<<<<<<< HEAD
			return NULL;
=======
			return "FAULT";
>>>>>>> origin/master
		}
		while((result=dbresults(dbprocess))!= NO_MORE_RESULTS)
		{
			if(result==SUCCEED)
			{
				dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)SrcUserId);
				dbbind(dbprocess,2,CHARBIND,(DBINT)0,(BYTE*)UserId);
				dbbind(dbprocess,3,VARYBINBIND,(DBINT)0,(BYTE*)Data);
				dbbind(dbprocess,4,CHARBIND,(DBINT)0,(BYTE*)SendTime);
				dbbind(dbprocess,5,INTBIND,(DBINT)0,(BYTE*)&mid);
				dbbind(dbprocess,6,INTBIND,(DBINT)0,(BYTE*)&Datalen);
				while(dbnextrow(dbprocess)!=NO_MORE_ROWS)
				{
					if(len<5120)
					{
						FinalD3desDecryption(deskey,Data+2,dData,Datalen);
						sprintf(tmp,"<message><sender>%s</sender><type>3</type><id>%d</id><typeid>%s</typeid><time>%s</time><context><![CDATA[%s]]></context></message>",SrcUserId,mid,UserId,SendTime,dData);
						len=strlen(xml);
						tlen=strlen(tmp);
						xml=(char*)realloc(xml,len+1+tlen);
						memset(xml+len,0,tlen+1);
						strcpy(xml+len,tmp);
						memset(tmp,0,tlen);
						memset(SrcUserId,0,32);
						memset(SendTime,0,20);
						memset(Data,0,4098);
						memset(dData,0,4096);
						memset(UserId,0,32);
						ret=1;
					}
				}
			}
		}
	}
	if(ret==1)
	{
		len=strlen(xml);
		xml=(char*)realloc(xml,len+12);
		memset(xml+len,0,12);
		strcat(xml,"</conpany>");
		tdbh->ctrling=0;
		return xml;
	}
	else
	{
		free(xml);
		tdbh->ctrling=0;
		return NULL;
	}
}

int get_group_mutil_user(char* uid,int gid,int type)
{
	char SQL[128]= {0};
	char UserId[32]= {0};
	if(type==CTRLGROUP)
		sprintf(SQL,"select UserId from group_customer_relation where GroupId=%d and UserId<>'%s'",gid,uid);
	else
		sprintf(SQL,"select UserId from mutil_customer_relation where MutilId=%d and UserId<>'%s'",gid,uid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
	if(dbresults(dbprocess) == SUCCEED)
	{
		dbbind(dbprocess,1,CHARBIND,(DBINT)0,(BYTE*)UserId);
		char* context=NULL;
		while(dbnextrow(dbprocess)!=NO_MORE_ROWS)
		{
			UL ul=get_point(user,UserId);
			if(NULL==ul)
			{
				memset(UserId,0,32);
				continue;
			}
			context=search_info(UserId,type);
			if(NULL!=context)
			{
				if(ul->fd!=-1&&ul->flag==1)
				{
					Zero_RE(ul->fd,context,strlen(context),1);
					ul->flag=0;
				}
			}
			if(context!=NULL&&strcmp("FALUT",context)!=0)
				free(context);
			memset(UserId,0,32);
		}
	}
	tdbh->ctrling=0;
	return 0;
}

/************************************************************************/
/* wh
/* 2014.07.02
/* PC版组织结构XML构造
/************************************************************************/

int GetOnlineCtms(int dpmid)
{
	if(org_stu != NULL && 0 != strcmp(org_stu, MO_XML_HEAD))
	{
		free(org_stu);
		org_stu=NULL;	//free了要把指针指向NULL，不然内存还是不会被释放的
	}
	g_nOrgStuLen = 0;
	org_stu = (char*)malloc(256 + 64);
	memset(org_stu, 0, 256 + 64);
	g_nOrgStuLen += 256;

	strcat(org_stu, MO_XML_HEAD);
	strcat(org_stu, "<version><company>");
	// 1、先根据传入的根部门ID获取其自身信息
	char sqltext[128] = {0};
	sprintf(sqltext, "SELECT [DpmName] FROM [department_info] WHERE [DpmId] = %d", dpmid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1 == CTRLDB(tdbh,sqltext))
	{
		tdbh->ctrling=0;
		return -1;
	}
	DBINT results = SUCCEED;
	char DpmName[128] = {0};
	char tmp[256] = {0};
	uint tlen=0;
	uint newlen=0;
	while((results = dbresults(dbprocess)) != NO_MORE_RESULTS)
	{
		if(results==SUCCEED)
		{
			dbbind(dbprocess, 1, CHARBIND, (DBCHAR)0, (BYTE*)DpmName);
			while(dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				// 添加根部门信息
				sprintf(tmp, "<department id=\"%d\" name=\"%s\" >", dpmid, DpmName);
				tlen = strlen(tmp);
				if(strlen(org_stu) + tlen > g_nOrgStuLen)
				{
					newlen = ((tlen/256) + 1)*256;
					org_stu = (char*)realloc(org_stu, g_nOrgStuLen + newlen + 32);
					g_nOrgStuLen += newlen;
				}
				strcat(org_stu, tmp);
				memset(tmp, 0, 256);
				memset(DpmName, 0, 128);
			}
		}
	}
	tdbh->ctrling=0;
	// 2、递归填充子节点信息
	if(GetSubDpmsAndWorkersForCvst(dpmid) != 0)
	{
		return -1;
	}
	strcat(org_stu, "</department></company></version>");
	return 0;
}

int GetSubDpmsAndWorkersForCvst(int dmpid)
{
	// 根据部门ID递归获取其子部门和直属员工
	// 获取所有子部门信息
	char sqltext[512] = {0};
	sprintf(sqltext, 
			"select department_info.DpmId, department_info.DpmName "\
			"from department_info left join department_department_relation "\
			"on department_info.DpmId = department_department_relation.SubordinateId "\
			"where department_department_relation.PrincipalId = %d order by department_department_relation.[DpmOrder]", dmpid);

/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;
/*****************************************************************************************/

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1 == CTRLDB(tdbh,sqltext))
	{
		tdbh->ctrling=0;
		return -1;
	}
	DBINT results = SUCCEED;
	int SubordinateId = 0;
	char DpmName[200] = {0};
	int nDmpCount = 0;
	int nMallocDmpCount = 10;
	struct _DPMNODE *pDpmNodes = (struct _DPMNODE*)malloc(nMallocDmpCount*sizeof(struct _DPMNODE));
	while((results = dbresults(dbprocess)) != NO_MORE_RESULTS)
	{
		if(results == SUCCEED)
		{
			dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&SubordinateId);
			dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)DpmName);
			while(dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				// 将直属部门信息记录下来
				struct _DPMNODE *pDpmNode = &pDpmNodes[nDmpCount++];
				memset(pDpmNode, 0, sizeof(struct _DPMNODE));
				pDpmNode->DpmId = SubordinateId;
				strcpy(pDpmNode->DpmName, DpmName);
				if(nDmpCount >= nMallocDmpCount)
				{
					nMallocDmpCount += 10;
					pDpmNodes = (struct _DPMNODE*)realloc(pDpmNodes, nMallocDmpCount*sizeof(struct _DPMNODE));
				}
				//SubordinateId = 0;	//不是字符串清不清都无所谓
				memset(DpmName, 0, 200);
			}
		}
	}
	tdbh->ctrling=0;
	// 从最后一个直属部门开始向上递归
	int i = 0;
	uint newlen=0;
	uint tlen=0;
	for(i = 0; i < nDmpCount; i++)
	{
		struct _DPMNODE *pDpmNode = &pDpmNodes[i];
		// 添加直属部门信息
		char tmp[256] = {0};
		sprintf(tmp, "<department id=\"%d\" name=\"%s\" >", pDpmNode->DpmId, pDpmNode->DpmName);
		tlen = strlen(tmp);
		if(strlen(org_stu) + tlen > g_nOrgStuLen)
		{
			newlen = ((tlen/256) + 1)*256;
			org_stu = (char*)realloc(org_stu, g_nOrgStuLen + newlen + 32);
			g_nOrgStuLen += newlen;
		}
		strcat(org_stu, tmp);
		// 在此递归填充子部门信息
		if(GetSubDpmsAndWorkersForCvst(pDpmNode->DpmId) != 0)
		{
			return -1;
		}
		// 添加结束符
		strcat(org_stu, "</department>");
	}
	free(pDpmNodes);
	pDpmNodes=NULL;	//free了要把指针指向NULL，不然内存还是不会被释放的

	// 获取所有直属员工信息
	sprintf(sqltext, 
			"select customer_info.UserId,"\
			"customer_info.Name,"\
			"customer_info.Sex,"\
			"customer_info.Mood "\
			"from customer_info left join department_customer_relation "\
			"on customer_info.UserId = department_customer_relation.UserId "\
			"where department_customer_relation.DpmId = %d ORDER BY department_customer_relation.[UserOrder]", dmpid);
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh1=SelectDbproc();
	if(NULL==tdbh1)
		return -1;
	DBPROCESS *dbprocess1=tdbh1->dbproc;
/*****************************************************************************************/
	dbcancel(dbprocess1);    //清除上次查询结果
	if(-1 == CTRLDB(tdbh1,sqltext))
	{
		tdbh1->ctrling=0;
		return -1;
	}
	char UserId[36] = {0};
	char Name[36] = {0};
	int Sex = 1;
	char Mood[392] = {0};
	uint tlen2=0;	//用同一个变量名，也不怕搞错啊(╯‵□′)╯︵┻━┻
	uint newlen2=0;
	while((results = dbresults(dbprocess1)) != NO_MORE_RESULTS)
	{
		if(results == SUCCEED)
		{
			dbbind(dbprocess1, 1, CHARBIND, (DBCHAR)0, (BYTE*)UserId);
			dbbind(dbprocess1, 2, CHARBIND, (DBCHAR)0, (BYTE*)Name);
			dbbind(dbprocess1, 3, INTBIND, (DBINT)0, (BYTE*)&Sex);
			dbbind(dbprocess1, 4, CHARBIND, (DBCHAR)0, (BYTE*)Mood);
			while(dbnextrow(dbprocess1) != NO_MORE_ROWS)
			{
				// 添加直属员工信息
				char tmp[512] = {0};
				sprintf(tmp, "<worker id=\"%s\" name=\"%s\" sex=\"%s\" mood=\"%s\"/>",
						UserId, Name, Sex ? "M" : "L", Mood);	//state反正都是0，客户端自己生成就可以了
				tlen2 = strlen(tmp);
				if(strlen(org_stu) + tlen2 > g_nOrgStuLen)
				{
					newlen2 = ((tlen2/256) + 1)*256;
					org_stu = (char*)realloc(org_stu, g_nOrgStuLen + newlen2 + 32);
					g_nOrgStuLen += newlen2;
				}
				strcat(org_stu, tmp);

				memset(UserId, 0, 36);
				memset(Name, 0, 36);
				//Sex = 1;	//不是字符串清不清都无所谓
				memset(Mood, 0, 392);
			}
		}
	}
	tdbh1->ctrling=0;
	return 0;
}

int check_user_photo(char* uid,char* md5)
{
	char SQL[128]={0};
	sprintf(SQL,"select id from customer_info where UserId='%s' and PhotoMd5='%s'",uid,md5);
/*****************************************新增连接池**************************************/
	DbprocHandler tdbh=SelectDbproc();
	if(NULL==tdbh)
		return -1;
	DBPROCESS *dbprocess=tdbh->dbproc;

	dbcancel(dbprocess);    //清除上次查询结果
	if(-1==CTRLDB(tdbh,SQL))
	{
		tdbh->ctrling=0;
		return -1;
	}
/*****************************************************************************************/

	int UID=0;
	DBINT res;
	if((res=dbresults(dbprocess))!=NO_MORE_RESULTS)
	{
		if(res==SUCCEED)
		{
			dbbind(dbprocess, 1, INTBIND, (DBINT)0, (BYTE*)&UID);
			if(dbnextrow(dbprocess) != NO_MORE_ROWS)
			{
				if(UID>0)
				{
					tdbh->ctrling=0;
					return 0;
				}
			}
		}
	}
	tdbh->ctrling=0;
	return -1;
}

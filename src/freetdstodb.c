#include "freetdstodb.h"

int InitData(char* server,char* user,char* passwd,char* dbname)		//初始化连接信息
{
	if(NULL==server||NULL==user||NULL==passwd||NULL==dbname)
	{
		return -1;
	}
	//将配置文件的数据库信息存入模块全局变量中
	DBSERVER=server;
	SQL_USER=user;
	PASSWD=passwd;
	SQL_DBNAME=dbname;
	return 0;
}

int ReConnect(DbprocHandler dbph)		//重新数据库连接
{
	//数据库
	//LOGINREC* loginrec = dblogin();
	//DBSETLUSER(loginrec, SQL_USER);
	//DBSETLPWD(loginrec, PASSWD);	
	if(dbph->dbproc!=FAIL)
	{
		dbclose(dbph->dbproc);
		dbph->dbproc=FAIL;
	}

	dbph->dbproc = dbopen(loginrec, DBSERVER);	//连接数据库
	//出错处理
	if(dbph->dbproc == FAIL)
	{
		char msg[80]={0};
		sprintf(msg,"dbh[%d] connect to MS SQL SERVER 2008 fail, exit",dbph->num);
		printf("%s\n",msg);
		writelog(msg);
		return -1;
	}
	else
	{
		char msg[80]={0};
		sprintf(msg,"dbh[%d] connect to MS SQL SERVER 2008 success",dbph->num);
		printf("%s\n",msg);
		writelog(msg);
	}
	//打开指定数据库
	if(dbuse(dbph->dbproc, SQL_DBNAME) == FAIL)
	{
		printf("Open database failed!\n");
		writelog("Open database failed");
		dbclose(dbph->dbproc);
		dbph->dbproc=FAIL;
		return -1;
	}
	else
	{
		printf("Open database success!\n");
		writelog("Open database success");
		dbph->flg=1;
		dbph->ctrling=0;
		if(initlink>linkcount)
			linkcount++;
		return 0;
	}
}

uint ConnectToDB()		//建立数据库连接
{
	uint link=0;	//统计连接成功的句柄

	dbinit();	//初始化db-lib
	loginrec = dblogin();
	DBSETLUSER(loginrec, SQL_USER);
	DBSETLPWD(loginrec, PASSWD);

	DbprocHandler dbh=NULL;

	int i=0;

	for(i=0;i<initlink;i++)
	{
		dbh=&dph[i];
		if(dbh->flg==0&&FAIL==dbh->dbproc)
		{
			dbh->dbproc = dbopen(loginrec, DBSERVER);	//连接数据库
			if(dbh->dbproc == FAIL)
			{
				char msg[80]={0};
				sprintf(msg,"dbh[%d] connect to MS SQL SERVER 2008 fail, exit",dbh->num);
				printf("%s\n",msg);
				writelog(msg);				
				continue;
			}
			else
			{
				char msg[80]={0};
				sprintf(msg,"dbh[%d] connect to MS SQL SERVER 2008 success",dbh->num);
				printf("%s\n",msg);
				writelog(msg);	
			}

			if(dbuse(dbh->dbproc, SQL_DBNAME) == FAIL)
			{
				printf("Open database failed!\n");
				writelog("Open database failed");
				dbclose(dbh->dbproc);
				dbh->dbproc=FAIL;
				continue;
			}
			else
			{
				printf("Open database success!\n");
				writelog("Open database success");
				//连接成功，初始化连接标记并统计连接数
				dbh->flg=1;
				dbh->ctrling=0;
				link++;
			}
		}
	}
	linkcount=link;	//标记连接数
	return link;
}

int CTRLDB(DbprocHandler dbh,char* SQL_CTRL)		//操作数据库
{
	dbfreebuf(dbh->dbproc);
	dbcmd(dbh->dbproc,SQL_CTRL);
	dbcmd(dbh->dbproc,TAIL);
	int dbflag=0;   //防止过分频繁操作数据库
	int ret=0;
	if(dbsqlexec(dbh->dbproc) == FAIL)
	{
		char msg[40]={0};
		sprintf(msg,"ctrl %d dbproc",dbh->num);
		printf("%s\n",msg);
		writelog(msg);	
		ret=-1;
		if(ReConnect(dbh)<0)
		{
			dbh->flg=0;
			if(linkcount>0)
				linkcount--;
		}
		else
		{
			char msg[40]={0};
			sprintf(msg,"ctrl reconnect %d dbproc",dbh->num);
			printf("%s\n",msg);
			writelog(msg);	
		}
	}
	dbflag=1;
	return ret;
}

void CloseConnection()		//关闭数据库，调用前先停止运行ReLinkThread线程
{
	int i=0;
	DbprocHandler dbh=NULL;
	for(i=0;i<initlink;i++)
	{
		dbh=&dph[i];
		if(dbh->flg==1)
		{
			if(dbh->dbproc!=FAIL)
			{
				dbclose(dbh->dbproc);
				dbh->dbproc=FAIL;
			}
			dbh->flg=0;
			char msg[40]={0};
			sprintf(msg,"close dbh[%d]",dbh->num);
			printf("%s\n",msg);
			writelog(msg);
		}
	}
	free(dph);
	dbloginfree(loginrec);
	dbexit();	//关闭db-lib
	if(pthread_mutex_destroy(&re_mutex)<0)
	{
		printf("pthread_mutex_destory re_mutex error\n");
		writelog("pthread_mutex_destory re_mutex error");
	}
}

void* ReLinkThread(void* arg)
{
	pthread_detach(pthread_self());
	DbprocHandler dbh=NULL;
	int i=0;
	pthread_mutex_lock(&re_mutex);
	for(i=0;i<initlink;i++)
	{
		dbh=&dph[i];
		if(dbh->flg==0)
		{
			if(ReConnect(dbh)==0)
			{
				char msg[40]={0};
				sprintf(msg,"reconnect database %d",dbh->num);
				printf("%s\n",msg);
				writelog(msg);
			}
		}
	}
	pthread_mutex_unlock(&re_mutex);
}

DbprocHandler SelectDbproc()		//选择连接
{
	int i=0,flg=0;
	DbprocHandler dbh=NULL,re_dbh=NULL;

	for(i=0;i<initlink;i++)
	{
		dbh=&dph[i];
		if(dbh->flg==1)
		{
			if((FAIL!=dbh->dbproc)&&(!dbdead(dbh->dbproc)))
			{
				if(dbh->ctrling==0)
				{
					dbh->ctrling=1;	//添加使用标记
					re_dbh=dbh;
					break;
				}
			}
			else
			{
				flg=1;
				dbh->flg=0;
				if(linkcount>0)
					linkcount--;
				char msg[48]={0};
				sprintf(msg,"select FAIL off %d dbproc!\n",dbh->num);
				printf("%s\n",msg);
				writelog(msg);
			}
		}
		else
		{
			flg=1;
			char msg[48]={0};
			sprintf(msg,"select flg off %d dbproc!\n",dbh->num);
			printf("%s\n",msg);
			writelog(msg);
			if(linkcount>0)
				linkcount--;
		}
	}
	if(flg)
	{
		pthread_t tid;
		if(pthread_create(&tid,NULL,(void*)ReLinkThread,NULL)<0)
		{
			printf("pthread_create relink error\n");
			writelog("pthread_create relink error");
		}
	}
	return re_dbh;
}

int DbprocInit(uint count)		//创建初始化连接句柄
{
	int i=0;
	dph=(DbprocHandler)malloc(sizeof(DbHandler)*count);
	if(dph==NULL)
	{
		perror("malloc error");
		writelog("freetds malloc");
		return -1;
	}
	DbprocHandler dbh=NULL;
	for(i=0;i<count;i++)
	{
		dbh=&dph[i];
		dbh->flg=0;
		dbh->ctrling=0;
		dbh->num=i;
		dbh->dbproc=FAIL;
	}
	initlink=count;
	if(pthread_mutex_init(&re_mutex,NULL)<0)
	{
		printf("pthread_mutex_init re_mutex error\n");
		writelog("pthread_mutex_init re_mutex error");
		return -1;
	}
	return 0;
}

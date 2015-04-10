#ifndef __SYBDB_H__
#define __SYBDB_H__

#include <stdio.h>
#include <sybfront.h> //freetds头文件 
#include <sybdb.h> //freetds头文件
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "log.h"

#define TAIL " set quoted_identifier on set ansi_warnings on set ansi_padding on set ansi_nulls on set concat_null_yields_null on"

typedef struct
{
	DBPROCESS* dbproc;	//数据库操作句柄
	int flg;	//数据库连接初始化标识
	int	ctrling;	//数据库连接使用标识
	uint num;	//编号
} DbHandler,*DbprocHandler;

char* DBSERVER;
char* SQL_USER;
char* PASSWD;
char* SQL_DBNAME;

LOGINREC* loginrec;

DbprocHandler dph;	//连接池句柄

uint linkcount;	//当前可用连接池数量
uint initlink;	//初始设定连接池数量

pthread_mutex_t re_mutex;	//重连线程互斥锁

void* ReLinkThread(void* arg);	//重连线程

int InitData(char* server,char* user,char* passwd,char* dbname);	//初始化连接信息

uint ConnectToDB();	//建立数据库连接

int ReConnect(DbprocHandler dbph);	//重新数据库连接

int CTRLDB(DbprocHandler dbph,char* SQL_CTRL);	//操作数据库

void CloseConnection();	//关闭数据库

DbprocHandler SelectDbproc();	//选择连接

int DbprocInit(uint count);	//创建初始化连接句柄

#endif

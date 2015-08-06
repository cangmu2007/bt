#include "head.h"
#include "conf.h"
#include <sybfront.h>
#include <sybdb.h>

int main(int argc,char *argv[])
{
	//读取配置文件
	struct config *cfg;

	if(argc!=2)
	{
		printf("Parameter error\n");
		exit(-1);
	}

	if(!(cfg=cfg_load_file(argv[1])))
	{
		printf("Reads the configuration file error!\n");
		return -1;
	}

	char* DBSERVER=cfg_getstr(cfg,"FreeTDS.DBSERVER");
	char* SQL_DBNAME=cfg_getstr(cfg,"FreeTDS.SQL_DBNAME");
	
	char* SQL_USER=cfg_getstr(cfg,"DB_LOGIN.SQL_USER");
	char* SQL_PASSWD=cfg_getstr(cfg,"DB_LOGIN.SQL_PASSWD");

	cfg_free(cfg);
	cfg=NULL;

	 //初始化db-library 
    dbinit(); 
        
    //初始化用户信息
    LOGINREC *loginrec = dblogin(); 
    DBSETLUSER(loginrec, SQL_USER);        
    DBSETLPWD(loginrec, SQL_PASSWD);
	
	//连接数据库 
    DBPROCESS *dbprocess = dbopen(loginrec, DBSERVER);
	if(dbprocess == FAIL) 
    { 
        printf("Conect to MS SQL SERVER 2008 fail, exit!\n"); 
        return -1;  
    } 
    printf("Connect to MS SQL SERVER 2008 success!\n"); 
        
    if(dbuse(dbprocess, SQL_DBNAME) == FAIL) 
        printf("Open database failed!\n"); 
    else 
        printf("Open database success!\n"); 

	//初始化SQL语句
	dbcmd(dbprocess,"delete from ctm_ctm_msg_mt where DATEDIFF(day,[SendTime],GETDATE()) > 7\n\
		delete from group_ctm_msg_mt where DATEDIFF(day,[SendTime],GETDATE()) > 7\n\
		delete from multi_ctm_msg_mt where DATEDIFF(day,[SendTime],GETDATE()) > 7");
	
	//执行语句
	if(dbsqlexec(dbprocess) == FAIL) 
    { 
        printf("delete tables error\n"); 
        return -1;  
    }	
	printf("Old conversation records have been deleted!\n");

	//清理资源
	dbclose(dbprocess);
	dbloginfree(loginrec); 
	dbexit();
	
	cfg_free(cfg);
	cfg=NULL;

	return 0;
}

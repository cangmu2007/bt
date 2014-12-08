#include "head.h"
#include "conf.h"

int main(int argc, char *argv[])
{
    /****************************主函数局部变量***********************/
    
	int i=0;
    int ret=-1;
    int max_cgi=-1;
    int new_fd=-1;
	int num = 1;
    pthread_t timer;
    struct sockaddr_un cgi_addr;

	/********************************************************************************************/

	/*******************************************初始化操作***************************************/

	if(argc!=2)
	{
		printf("Parameter error\n");
		exit(-1);
	}
	
	struct config *cfg;
	if(!(cfg = cfg_load_file(argv[1])))
	{
		printf("Reads the configuration file error!\n");
		exit(-1);
	}

	DBSERVER=cfg_getstr(cfg,"FreeTDS.DBSERVER");  //数据库服务器连接地址
	SQL_DBNAME=cfg_getstr(cfg,"FreeTDS.SQL_DBNAME");  //数据库名

	MAX_DATA=cfg_getnum(cfg,"DB_LOGIN.MAX_DATA");
	SQL_USER=cfg_getstr(cfg,"DB_LOGIN.SQL_USER");   //用户名
	SQL_PASSWD=cfg_getstr(cfg,"DB_LOGIN.SQL_PASSWD"); //密码
	SQL_LINK_COUNT=cfg_getnum(cfg,"DB_LOGIN.SQL_LINK_COUNT");	//数据库连接池
	CONPANY_ID=cfg_getnum(cfg,"DB_LOGIN.CONPANY_ID");    //公司ID

	MI_ADDR=cfg_getstr(cfg,"MI_CONTECT.MI_ADDR"); //中间件IP
	MI_PORT=cfg_getnum(cfg,"MI_CONTECT.MI_PORT");    //中间件端口

	MAX_CGI_LINK=cfg_getnum(cfg,"CGI_SET.MAX_CGI_LINK");   //CGI最大连接数
	UNIX_PATH=cfg_getstr(cfg,"CGI_SET.UNIX_PATH");    //域套接字连接符路径(用户必须设置有读写权限的目录内，套接口的名称必须符合文件名命名规则且不能有后缀，该变量和CGI头文件中的同名宏必须相同)
	LOG_PATH=cfg_getstr(cfg,"CGI_SET.LOG_PATH");	//日志路径

	if(daemon(1, 1) < 0)  
    {  
        perror("error daemon"); 
        exit(-1);  
    }

	if(openlog(LOG_PATH)<0)	//打开日志
	{
		exit(-1);
	}
	
	user=(UL)malloc(sizeof(User_Linking));  //初始化用户链表
    user->next=NULL;
    org_stu=NULL;   //初始化组织结构
    tmp_stu=NULL;
    PC_OL=NULL_OL();
    MO_OL=NULL_OL();

	if(InitData(DBSERVER,SQL_USER,SQL_PASSWD,SQL_DBNAME)<0)	//初始化数据库连接
	{
		printf("InitData error!\n");
		exit(-1);
	}

    /***************************************************************************************************/

    /*if(pthread_mutex_init(&mutex_sql,NULL)<0)   //初始化互斥锁
    {
        printf("pthread_mutex_init ol error\n");
        exit(-1);
    }*/

	if(DbprocInit(SQL_LINK_COUNT)<0)	//连接数据库
	{
		printf("DbprocInit error\n");
		writelog("DbprocInit error");
		exit(-1);
	}

	SQL_LINK_COUNT=ConnectToDB(dph);
	if(SQL_LINK_COUNT<=0)
	{
		printf("Could not establish connection\n");
		writelog("Could not establish connection");
        exit(-1);
	}

    /*if(-1 == ConnectToDB(DBSERVER,SQL_USER,SQL_PASSWD,SQL_DBNAME))	//连接数据库
    {
        printf("Could not establish connection\n");
        exit(-1);
    }*/

    /****************************************************************************************************/

    /************************************************创建于中间件交互线程***************************************/

    if(Init_Mi_TCP(MI_ADDR,MI_PORT)<0)
	{
		exit(-1);
	}

	if(pthread_create(&check_mi,NULL,(void*)CHECK_MI_LINK,NULL)<0)
	{
		printf("pthread_create check_mi_link error\n");
		writelog("pthread_create check_mi_link error");
		exit(-1);
	}

    /*************************************************************************************************************/

    /**********************************CGI的域套接字初始化************************/

	if(pthread_mutex_init(&mutex_cgi,NULL)<0) //初始化互斥锁
    {
        printf("pthread_mutex_init cgi error\n");
		writelog("pthread_mutex_init cgi error");
        exit(-1);
    }

    if((cgi_fd = socket(PF_UNIX,SOCK_STREAM,0))<0)  //域套接字的socket
    {
        perror("TCP cgi socket");
		writelog("TCP cgi socket");
        exit(-1);
    }
	setnonblocking(cgi_fd);

    /*************************************epoll方法定义，windows下不支持**********************/
#ifdef LINUX   
    struct epoll_event ev={0},events[20];
    int epfd=epoll_create1(0);
    if(-1==epfd)
    {
        perror("epoll_create1 error");
		writelog("epoll_create1 error");
        exit(-1);
    }
    ev.data.fd=cgi_fd;
    ev.events=EPOLLIN|EPOLLET;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, cgi_fd, &ev)<0)
    {
        perror("epoll_ctl error");
		writelog("epoll_ctl error");
        exit(-1);
    }
    int nfds=-1;
    /*****************************************************************************************/

    /*************************************传统SELECT方式，通用于windows*************************/
#else
	struct pollfd master[MAX_CGI_LINK+1];
	for(i = 0; i < MAX_CGI_LINK+1; i++)
		master[i].fd = -1;

	master[0].fd = cgi_fd;
	master[0].events = POLLIN;

	int tfd=-1;
	int maxcli=-1;

    //fd_set rd_set;
#endif
    /*****************************************************************************************/

    unlink(UNIX_PATH);  //删除原有套接字连接

    memset(&cgi_addr,0,sizeof(cgi_addr));
    cgi_addr.sun_family = AF_UNIX;
    strcpy(cgi_addr.sun_path, UNIX_PATH);

    if(bind(cgi_fd, (struct sockaddr *)&cgi_addr, sizeof(struct sockaddr_un))<0)
    {
        perror("TCP cgi bind");
		writelog("TCP cgi bind");
        exit(-1);
    }

    if(chmod(UNIX_PATH,0777)<0) //写域套接字的权限
    {
        perror("chmod");
		writelog("chmod");
        exit(-1);
    }

    if(listen(cgi_fd, MAX_CGI_LINK)<0)
    {
        perror("TCP cgi listen");
		writelog("TCP cgi listen");
        exit(-1);
    }

    /***********************************************************************************/

    if(pthread_create(&timer, NULL, (void*)Flush_CGI, NULL)<0)  //创建保活线程
    {
        printf("pthread_create timer\n");
		writelog("pthread_create timer");
        exit(-1);
    }

    printf("Initialization is ready, start listening now connect business users......\n");

	writelog("btserver start");
    for(;;)
    {
        /*************************epoll方法，需要Linux2.6.18以上支持*************/
#ifdef LINUX
        if((nfds=epoll_wait(epfd,events,num,-1))<0)
        {
            //if(errno==EINTR)    //调试时使用，调试结束后必须注释
                //continue;

            perror("epoll_wait error");
			writelog("epoll_wait error");
            exit(-1);
        }

        for(i=0; i<nfds; ++i)
        {
            if(events[i].data.fd==cgi_fd)
            {
                if((new_fd=accept(cgi_fd, NULL, NULL))<0)
                {
                    perror("TCP cgi accept");
					writelog("TCP cgi accept error");

                    //重建CGI域套接字
                    close(cgi_fd);
                    sleep(2);
                    if((cgi_fd = socket(PF_UNIX,SOCK_STREAM,0))<0)  //域套接字的socket
                    {
                        perror("TCP cgi resocket");
						writelog("TCP cgi resocket");
                        break;
                    }

                    setnonblocking(cgi_fd);

                    unlink(UNIX_PATH);  //删除原有套接字连接

                    memset(&cgi_addr,0,sizeof(cgi_addr));
                    cgi_addr.sun_family = AF_UNIX;
                    strcpy(cgi_addr.sun_path, UNIX_PATH);

                    if(bind(cgi_fd, (struct sockaddr *)&cgi_addr, sizeof(struct sockaddr_un))<0)
                    {
                        perror("TCP cgi rebind");
						writelog("TCP cgi rebind");
                        break;
                    }

                    if(chmod(UNIX_PATH,0777)<0)
                    {
                        perror("chmod error");
						writelog("chmod error");
                        exit(-1);
                    }

                    if(listen(cgi_fd, MAX_CGI_LINK)<0)
                    {
                        perror("TCP cgi relisten");
						writelog("TCP cgi relisten");
                        break;
                    }

					DeleteULList(user);
                    continue;
                }

                setnonblocking(new_fd);

                ev.data.fd=new_fd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &ev);
                num++;
            }
            else if(events[i].events & EPOLLIN)
            {
                if(events[i].data.fd<0)
                    continue;
                pthread_t tid;
                if(pthread_create(&tid,NULL,(void*)CGI_Link,(void*)(&events[i].data.fd))<0)
                {
                    perror("pthread_create epoll");
					writelog("pthread_create epoll");
                    close(events[i].data.fd);
                }
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                num--;
                //events[i].data.fd=-1;
            }
            else if(events[i].events&EPOLLOUT)
            {
                //异步发送，这里用不到
            }
        }
        /****************************************************************************************/
#else
		if(poll(master, num, -1) < 0)
		{
			perror("poll error");
			writelog("poll error");
			exit(-1);
		}
		if(master[0].revents & POLLIN)
		{
			if((new_fd = accept(cgi_fd, NULL, NULL))<0)
			{
				perror("TCP cgi accept");
				writelog("TCP cgi accept");

                //重建CGI域套接字
                close(cgi_fd);
                sleep(2);
                if((cgi_fd = socket(PF_UNIX,SOCK_STREAM,0))<0)  //域套接字的socket
                {
                    perror("TCP cgi resocket");
					writelog("TCP cgi resocket");
                    break;
                }

                unlink(UNIX_PATH);  //删除原有套接字连接

                memset(&cgi_addr,0,sizeof(cgi_addr));
                cgi_addr.sun_family = AF_UNIX;
                strcpy(cgi_addr.sun_path, UNIX_PATH);

                if(bind(cgi_fd, (struct sockaddr *)&cgi_addr, sizeof(struct sockaddr_un))<0)
                {
                    perror("TCP cgi rebind");
					writelog("TCP cgi rebind");
                    break;
                }

                if(chmod(UNIX_PATH,0777)<0)
                {
                    perror("chmod error");
					writelog("chmod error");
                    exit(-1);
                }

                if(listen(cgi_fd, MAX_CGI_LINK)<0)
                {
                    perror("TCP cgi relisten");
					writelog("TCP cgi relisten");
                    break;
                }

				DeleteULList(user);
                continue;
			}
			setnonblocking(new_fd);

			for(i = 0; i < MAX_CGI_LINK+1; i++)
			{
				if(master[i].fd < 0)
				{
					master[i].fd = new_fd;
					master[i].events = POLLIN;
					if(i > maxcli)
						maxcli = i;
					num ++;
					break;
				}
			}
		}

		for(i=1; i<=maxcli; i++)
		{
			if(master[i].revents & POLLIN)
			{
				pthread_t tid;
				tfd=master[i].fd;
				master[i].fd=-1;
				if(pthread_create(&tid,NULL,(void*)CGI_Link,(void*)(&tfd))<0)
				{
					perror("pthread_create poll");
					writelog("pthread_create poll");
					close(tfd);
				}
				num--;
			}
		}
#endif
    }

    Exit_Mi_TCP();

	if(pthread_cancel(check_mi)<0)  //关闭中间件检查线程
    {
        printf("pthread_cancle check_mi\n");
		writelog("pthread_cancle check_mi");
        return -1;
    }

	if(pthread_join(check_mi,NULL)<0) //阻塞主线程
    {
        printf("pthread_join check_mi\n");
		writelog("pthread_join check_mi");
        return -1;
    }

	if(pthread_mutex_destroy(&mutex_cgi)<0)  //销毁互斥锁
    {
        printf("pthread_mutex_destroy(ol)");
		writelog("pthread_mutex_destroy(ol)");
        exit(-1);
    }

	/*if(pthread_mutex_destroy(&mutex_sql)<0) //销毁互斥锁
    {
        printf("pthread_mutex_destroy(ol)");
        exit(-1);
    }*/

	CloseConnection(dph);	//断开数据库
    //CloseConnection();  //断开数据库
	closelog();
    return 0;
}

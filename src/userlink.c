//用户在线链表操作

#include "head.h"
static pthread_mutex_t mutex_ul=PTHREAD_MUTEX_INITIALIZER;	//用户链表处理线程锁
static pthread_mutex_t mutex_ol=PTHREAD_MUTEX_INITIALIZER;	//移动在线用户刷新线程锁

void DeleteList(IL head)
{
    IL DElem, next;
    DElem = head->next;
    while(DElem)
    {
        next = DElem->next;
        free(DElem);
        DElem = next;
    }
}

void DeleteULList(UL head)
{
    UL DElem, next;
    DElem = head->next;
    while(DElem)
    {
        next = DElem->next;
		DeleteList(DElem->il);
		free(DElem->il);
		if(DElem->fd!=-1)
		{
			close(fd);
			fd=-1;
		}
        free(DElem);
        DElem = next;
    }
}

void insert_imf(IL head,char* context,uint len)
{
    IL p=head;
	pthread_mutex_lock(&mutex_ul);
    while(p&&p->next)
        p=p->next;
    IL new_ele=(IL)malloc(sizeof(IMF_LIST)+len+1);
    memset(new_ele,0,sizeof(IMF_LIST)+len+1);
    new_ele->len=len;
    memcpy(new_ele->context,context,len);
    new_ele->next=NULL;
    p->next=new_ele;
	pthread_mutex_unlock(&mutex_ul);
}

IL get_imf(IL head)
{
    return head->next;
}

int delete_imf(IL head,IL uimf)
{
	int ret;
	IL q=uimf,p=head;
	pthread_mutex_lock(&mutex_ul);
    while(p -> next != uimf)
	{
        p=p->next;
		if(p==NULL)
		{
			ret=-1;
			goto _re;
		}
	}
    if(q->next==NULL)
    {
        p->next=NULL;
        free(q);
        q=NULL;
    }
    else
    {
        p->next=q->next;
        free(q);
        q=NULL;
    }
	ret=0;
_re:
	pthread_mutex_unlock(&mutex_ul);
    return ret;
}

UL get_point(UL head,char* loginer) //获取用户在线链表节点
{
    UL p=head->next;
    while(p)
    {
        if(0==strcmp(loginer,p->id))
            return p;
        p=p->next;
    }
    return NULL;
}

UL get_point_fd(UL head,int fd) //获取用户在线链表节点
{
    UL p=head->next;
    while(p)
    {
        if(p->fd==fd)
            return p;
        p=p->next;
    }
    return NULL;
}

int update_point_fd(UL head,char* loginer,int fd)   //修改用户在线链表节点的文件描述符
{
    UL p=head->next;
	int re=-1;
	pthread_mutex_lock(&mutex_ul);
    while(p)
    {
        if(0==strcmp(loginer,p->id))
        {
            if(p->fd!=-1)
                re=close(p->fd);
            printf("%s close fd=%d and up fd=%d,result=%d\n",p->id,p->fd,fd,re);
            p->fd=fd;
            re=0;
			break;
        }
        p=p->next;
    }
	pthread_mutex_unlock(&mutex_ul);
    return re;
}

/*int update_point_pth(UL head,int fd,pthread_t time) //修改用户在线链表节点的线程描述符
{
    UL p=head->next;
    while(p)
    {
        if(fd==p->fd)
        {
            p->time=time;
            return 0;
        }
        p=p->next;
    }
    return -1;
}*/

int delete_point_log(UL head,UL uid) //用户在线链表删除节点
{
	int ret=-1;
    UL q=uid,p=head;
	pthread_mutex_lock(&mutex_ul);
    while(p -> next != uid)
	{
        p=p->next;
		if(p==NULL)
		{
			ret=-1;
			goto _re;
		}
	}
    if(q->next==NULL)
    {
        p->next=NULL;
        printf("user %s outline!\n",q->id);
        if(q->il!=NULL)
        {
            DeleteList(q->il);
            free(q->il);
            q->il=NULL;
        }
        free(q);
        q=NULL;
    }
    else
    {
        p->next=q->next;
        printf("user %s outline!\n",q->id);
        if(q->il!=NULL)
        {
            DeleteList(q->il);
            free(q->il);
            q->il=NULL;
        }
        free(q);
        q=NULL;
    }
	ret=0;
_re:
	pthread_mutex_unlock(&mutex_ul);
    return ret;
}

int insert_point(UL head,char* loginer,int fd)  //用户在线链表增加节点
{
    UL s,q=head;
	int ret=0;
	pthread_mutex_lock(&mutex_ul);
    while(q->next)
    {
        if(0==strcmp(q->next->id,loginer))   //已经在线
		{
			ret=1;
            break;
		}
        q=q->next;
    }
	if(ret==0)
	{
		s=(UL)malloc(sizeof(User_Linking));
		memset(s,0,sizeof(User_Linking));
		strcpy(s->id,loginer);
		s->fd=fd;
		s->next=NULL;
		IL il=(IL)malloc(sizeof(IMF_LIST));
		memset(il,0,sizeof(IMF_LIST));
		il->next=NULL;
		s->il=il;
		s->flag=0;
		q->next=s;
		printf("user %s online!\n",s->id);
	}
	pthread_mutex_unlock(&mutex_ul);
    return ret;
}

int get_len(UL head)    //获取用户在线链表节点数量
{
    UL p=head->next ;
    int len=0;
    while (p)
    {
        len++;
        p=p->next;
    }
    return len;
}

char* NULL_OL()
{
    char* init_ol=(char*)malloc(56); //初始化在线列表
    memset(init_ol,0,56);
    strcpy(init_ol,MO_XML_HEAD);
    strcat(init_ol,"<online></online>");
    return init_ol;
}

void flush_list(UL head)
{
    char* list=(char*)malloc(60);
    memset(list,0,60);
    strcpy(list,MO_XML_HEAD);
    strcat(list,"<online type=\"2\">");
    char temp[48]= {0};
    UL p=head->next;
    uint tlen,len=strlen(list);
    while(p)
    {
        sprintf(temp,"<user>%s</user>",p->id);
        tlen=strlen(temp);
        list=(char*)realloc(list,len+tlen+1);
        memset(list+len,0,tlen+1);
        strcpy(list+len,temp);
        len=len+tlen;
        memset(temp,0,48);
        p=p->next;
    }
    list=(char*)realloc(list,len+10);
	memset(list+len,0,10);
    strcpy(list+len,"</online>");
    pthread_mutex_lock(&mutex_ol);  //上锁
    if(NULL!=MO_OL)
        free(MO_OL);
    MO_OL=list;
    pthread_mutex_unlock(&mutex_ol);    //解锁
}

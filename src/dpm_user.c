#include "dpm_user.h"

static char* xml=NULL;
static Dpm thead=NULL;

Users init_user()
{
	Users uhead=(Users)malloc(sizeof(UserNode));
	if(NULL==uhead)
	{
		return NULL;
	}
	memset(uhead,0,sizeof(UserNode));
	uhead->next=NULL;
	return uhead;
}

int insert_user(Users head,char* uname,char* rname,int sex,int sort_id)
{
	Users p=head;
	while(p&&p->next)
	{
		if(sort_id<p->next->sort_id)
		{
			break;
		}
		p=p->next;
	}
	Users new_ele=(Users)malloc(sizeof(UserNode));
	if(NULL==new_ele)
	{
		return -1;
	}
	memset(new_ele,0,sizeof(UserNode));
	strcpy(new_ele->username,uname);
	strcpy(new_ele->real_name,rname);
	new_ele->sex=sex;
	new_ele->sort_id=sort_id;
	new_ele->next=p->next;
	p->next=new_ele;
	return 0;
}

void delete_user(Users head)
{
	if(NULL==head)
		return;
	Users DElem,next;
	DElem = head->next;
	while(DElem)
	{
		next = DElem->next;
		free(DElem);
		DElem = next;
	}
}

Dpm search_dpm(char* ud_id,Dpm zhead)
{
	if(NULL==zhead)
	{
		return NULL;
	}
	Dpm p=zhead,r;

	if(NULL==ud_id)
	{
		return p;
	}

	p=p->next;
	while(p)
	{
		if(strcmp(ud_id,p->dpmid)==0)
		{
			return p->node;
		}
		else
		{
			if(p->node->next)
			{
				r=search_dpm(ud_id,p->node);
				if(NULL!=r)
					return r;
			}
			p=p->next;
		}
	}
	return NULL;
}

int insert_dpm(Dpm dhead,char* up_id,char* id,char* name,int sort_id,Users uhead)
{
	Dpm p=search_dpm(up_id,dhead);
	if(NULL==p)
	{
		return -1;
	}

	//子部门头指针
	Dpm zele=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==zele)
	{
		return -1;
	}
	memset(zele,0,sizeof(DpmNode));
	zele->next=NULL;

	Dpm new_ele=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==new_ele)
	{
		free(zele);
		return -1;
	}
	memset(new_ele,0,sizeof(DpmNode));
	if(up_id!=NULL)
		strcpy(new_ele->updpmid,up_id);
	strcpy(new_ele->dpmid,id);
	strcpy(new_ele->dpmname,name);
	new_ele->node=zele;
	new_ele->sort_id=sort_id;
	new_ele->user=uhead;

	while(p&&p->next)
	{
		if(sort_id<p->next->sort_id)
		{
			break;
		}
		p=p->next;
	}
	new_ele->next=p->next;
	p->next=new_ele;

	return 0;
}

int insert_temp_dpm(char* up_id,char* id,char* name,int sort_id,Users uhead)
{
	Dpm p=thead;
	while(p&&p->next)
		p=p->next;

	//子部门头指针
	Dpm zele=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==zele)
	{
		return -1;
	}
	memset(zele,0,sizeof(DpmNode));
	zele->next=NULL;

	Dpm new_ele=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==new_ele)
	{
		free(zele);
		return -1;
	}
	memset(new_ele,0,sizeof(DpmNode));
	if(up_id!=NULL)
		strcpy(new_ele->updpmid,up_id);
	strcpy(new_ele->dpmname,name);
	strcpy(new_ele->dpmid,id);
	new_ele->node=zele;
	new_ele->sort_id=sort_id;
	new_ele->user=uhead;
	new_ele->next=NULL;
	p->next=new_ele;
	return 0;
}

int temp_dpm2dpm_list(Dpm head,Dpm ele)
{
	if(NULL==ele)
	{
		return -1;
	}
	Dpm p=search_dpm(ele->updpmid,head);
	if(NULL==p)
	{
		return -1;
	}
	while(p&&p->next)
	{
		if(ele->sort_id<p->next->sort_id)
		{
			break;
		}
		p=p->next;
	}
	ele->next=p->next;
	p->next=ele;
	return 0;
}

int insert_temp2dpm(Dpm head)
{
	Dpm p=head,q=thead,t=NULL;
	if(thead==NULL)
		return -1;
	while(q->next)
	{
		t=q->next->next;
		if(temp_dpm2dpm_list(p,q->next)<0)
		{
			free(q->next);
		}
		q->next=t;
	}
	return 0;
}

void delete_dpm(Dpm zhead)
{
	if(NULL==zhead)
	{
		return;
	}

	Dpm DElem,next;
	DElem = zhead->next;
	while(DElem)
	{
		delete_user(DElem->user);
		free(DElem->user);
		DElem->user=NULL;
		delete_dpm(DElem->node);
		free(DElem->node);
		next = DElem->next;
		free(DElem);
		DElem = next;
	}
}

Dpm init_dpm()
{
	Dpm dhead=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==dhead)
	{
		return NULL;
	}
	memset(dhead,0,sizeof(DpmNode));
	dhead->next=NULL;
	
	thead=(Dpm)malloc(sizeof(DpmNode));
	if(NULL==thead)
	{
		free(dhead);
		return NULL;
	}
	memset(thead,0,sizeof(DpmNode));
	dhead->next=NULL;

	return dhead;
}

void clean_dpm(Dpm dhead)
{
	free(dhead);
	dhead=NULL;
	free(thead);
	thead=NULL;
}

int dpmuser2xml(Dpm head)
{
	if(NULL==head)
	{
		return -1;
	}
	Dpm p=head->next;
	Users u=NULL;
	char dpmtmp[256] = {0};
	char usrtmp[128] = {0};
	uint tmplen=0,len=strlen(xml);
	char* temp=NULL;
	while(p)
	{
		sprintf(dpmtmp, "<department id=\"%s\" name=\"%s\">", p->dpmid, p->dpmname);
		tmplen=strlen(dpmtmp);
		temp=(char*)realloc(xml,len+tmplen+1);
		if(NULL==temp)
		{
			return -1;
		}
		xml=temp;
		memset(xml+len,0,tmplen+1);
		strcpy(xml+len,dpmtmp);
		memset(dpmtmp,0,256);
		len=len+tmplen;

		//子部门
		if(dpmuser2xml(p->node)<0)
		{
			return -1;
		}

		len=strlen(xml);
		if(NULL!=p->user)
		{
			u=p->user->next;
			while(u)
			{
				sprintf(usrtmp,"<worker id=\"%s\" name=\"%s\" sex=\"%c\"/>",u->username,u->real_name,u->sex!=2?'M':'L');
				tmplen=strlen(usrtmp);
				temp=(char*)realloc(xml,len+tmplen+1);
				if(NULL==temp)
				{
					return -1;
				}
				xml=temp;
				memset(xml+len,0,tmplen+1);
				strcpy(xml+len,usrtmp);
				memset(usrtmp,0,128);
				len=len+tmplen;
				u=u->next;
			}
		}

		temp=(char*)realloc(xml,len+13+1);
		if(NULL==temp)
		{
			return -1;
		}
		xml=temp;
		memset(xml+len,0,13+1);
		strcpy(xml+len,"</department>");
		len=len+13;
		p=p->next;
	}
	//printf("%s\n",xml);
	return 0;
}

char* struct2xml(Dpm head)
{
	/*if(NULL!=xml)
	{
		free(xml);
		xml=NULL;
	}*/
	xml=(char*)malloc(52);
	if(NULL==xml)
	{
		return xml;
	}
	memset(xml,0,52);
	strcat(xml,MO_XML_HEAD);
	strcat(xml,"<company>");

	if(dpmuser2xml(head)<0)
	{
		free(xml);
		xml=NULL;
		return NULL;
	}
	
	uint len=strlen(xml);
	char* tempxml=(char*)realloc(xml,len+13+1);
	if(NULL==tempxml)
	{
		free(xml);
		xml=NULL;
		return NULL;
	}
	xml=tempxml;
	memset(xml+len,0,13+1);
	strcpy(xml+len,"</company>");
	return xml;
}
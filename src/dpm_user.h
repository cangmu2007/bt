#ifndef __DPM_H__
#define __DPM_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MO_XML_HEAD "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

typedef struct users
{
	char username[48];	//用户ID
	char real_name[48];	//用户名
	int sex;	//性别1为男，其它为女
	int sort_id;
	struct users* next;
}*Users,UserNode;

typedef struct dpm
{
	char dpmid[48];	//部门ID
	char updpmid[48];	//上级部门
	char dpmname[192];	//部门名称
	struct dpm* node;	//子部门链表头
	Users user;	//部门人员
	int sort_id;
	struct dpm* next;	//同级部门链表
}*Dpm,DpmNode;

//初始化部门内用户链表头
Users init_user();

//添加用户
int insert_user(Users head,char* uname,char* rname,int sex,int sort_id);

//删除用户链表
void delete_user(Users head);

//初始化部门链表头
Dpm init_dpm();

//删除部门链表头
void clean_dpm(Dpm dhead);

//创建部门
int insert_dpm(Dpm dhead,char* up_id,char* id,char* name,int sort_id,Users uhead);

//创建缓存部门
int insert_temp_dpm(char* up_id,char* id,char* name,int sort_id,Users uhead);

//删除（子）部门链表
void delete_dpm(Dpm head);

//部门结构体转换成XML字符串
char* struct2xml(Dpm head);

//将缓存部门逐个添加到部门树中
int insert_temp2dpm(Dpm head);

#endif
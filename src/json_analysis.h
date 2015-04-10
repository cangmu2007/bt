#ifndef __JSON_C__
#define __JSON_C__

#include <stdio.h>
#include <string.h>
#include "json-c/json.h"
#include "dpm_user.h"

//出错提示
#define ERROR_1 "server_error"
#define ERROR_2 "invalid_request"
#define ERROR_3 "invalid_client"
#define ERROR_4 "unauthorized_client"
#define ERROR_5 "unsupported_response_type"
#define ERROR_6 "unsupport_grant_type"
#define ERROR_7 "invalid_user"
#define ERROR_8 "invalid_grant"
#define ERROR_9 "access_denied"

//服务端保存的令牌信息
typedef struct
{
	char access_token[36];	//访问令牌
	//char token_type[8];	//访问令牌类型
	//uint expires_in;	//访问令牌过期时间
	char refresh_token[36];	//更新令牌
	char user_name[48];	//令牌的用户名
	//char scope[20];	//令牌的权限列表
	//char return_uri[128];	//原始的访问地址
}ReToken,*Token;

//用户信息保存结构
typedef struct 
{
	char Nickname[48];	//昵称
	char real_name[48];	//真实姓名
	char personal_web_uri[128];	//个人主页地址
	char birthday[20];	//出生时间
	char qq_number[20];	//QQ号
	int sex;	//性别
	char main_post[48];	//主岗位ID
	char posts[48];	//岗位ID
	char Phone[48];	//电话
	char Mobile[48];	//手机
	char Mail[48];	//电子邮件
	char Photo[48];	//头像图片ID
	char Sign[384];	//签名
}UserInfo,*UI;

//解析组织结构
char* analysis_schema(char* str);

//解析用户个人信息
int analysis_userinfo(char* str,UI ui);

//解析令牌信息
int analysis_token(char* str,Token tok);

//解析令牌出错信息
int analysis_error(char* str,char* out);

//解析资源出错信息
int analysis_res_error(char* str);

#endif
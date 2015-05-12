#include "json_analysis.h"
#include "encode_and_decode.h"

char* dpm_json2xml(json_object* obj)
{
	uint dpm_len=json_object_array_length(obj);
	if(dpm_len==0)
	{
		return NULL;
	}

	int i,j;
	json_object* tempdpm=NULL;
	Dpm dpmhead=init_dpm();
	int flag=0;
	for(i=0;i<dpm_len;i++)
	{
		tempdpm=json_object_array_get_idx(obj,i);
		if(NULL==tempdpm)
		{
			continue;
		}

		//用户节点构建
		json_object* tempusers=NULL;
		Users uhead=NULL;
		if(json_object_object_get_ex(tempdpm,"users",&tempusers))
		{
			uint user_len=json_object_array_length(tempusers);
			if(user_len>0)
			{
				json_object* tempuser=NULL;
				uhead=init_user();
				if(uhead!=NULL)
				{
					for(j=0;j<user_len;j++)
					{
						tempuser=json_object_array_get_idx(tempusers,j);
						if(NULL==tempuser)
						{
							continue;
						}
						char* username=NULL;
						char* real_name=NULL;
						int sex=0;
						int sortid=0;
						json_object_object_foreach(tempuser,key,value)
						{
							if(strcmp(key,"username")==0)
							{
								username=(char*)json_object_get_string(value);
							}
							else if(strcmp(key,"real_name")==0)
							{
								real_name=(char*)json_object_get_string(value);
							}
							else if(strcmp(key,"sex")==0)
							{
								sex=json_object_get_int(value);
							}
							else if(strcmp(key,"sort_id")==0)
							{
								sortid=json_object_get_int(value);
							}
						}
						insert_user(uhead,username,real_name,sex,sortid);
					}
				}
			}
		}

		//部门节点构建
		char* dpmid=NULL;
		char* updpmid=NULL;
		char* dpmname=NULL;
		int sid=0;
		json_object_object_foreach(tempdpm,key,value)
		{
			if(strcmp(key,"department_id")==0)
			{
				dpmid=(char*)json_object_get_string(value);
			}
			else if(strcmp(key,"up_department_id")==0)
			{
				updpmid=(char*)json_object_get_string(value);
			}
			else if(strcmp(key,"sort_id")==0)
			{
				sid=json_object_get_int(value);
			}
			else if(strcmp(key,"department_name")==0)
			{
				dpmname=(char*)json_object_get_string(value);
			}
		}
		if(insert_dpm(dpmhead,updpmid,dpmid,dpmname,sid,uhead)<0)
		{
			if(insert_temp_dpm(updpmid,dpmid,dpmname,sid,uhead)==0)
			{
				flag=1;
			}
			//free(uhead);
		}
	}

	if(flag)
	{
		insert_temp2dpm(dpmhead);
	}

	char *results=struct2xml(dpmhead);
	delete_dpm(dpmhead);
	clean_dpm(dpmhead);
	return results;
}

char* analysis_schema(char* str)
{
	json_object *pobj = json_tokener_parse(str);
	//json_object *pobj = json_object_from_file(str);
	if(NULL==pobj)
	{
		return NULL;
	}
	json_object *obj=NULL;
	char* results=NULL;
	if(json_object_object_get_ex(pobj,"departments",&obj))
	{
		results=dpm_json2xml(obj);
	}
	json_object_put(pobj);
	return results;
}

int analysis_userinfo(char* str,UI ui)
{
	json_object *pobj = json_tokener_parse(str);
	//json_object *pobj = json_object_from_file(str);
	if(NULL==pobj)
		return -1;

	char *val=NULL;
	json_object_object_foreach(pobj,key,value)
	{ 
		val=(char*)json_object_get_string(value);
		if(NULL!=val)
		{
			if(strcmp(key,"nick_name")==0)
				strcpy(ui->Nickname,json_object_get_string(value));
			else if(strcmp(key,"real_name")==0)
				strcpy(ui->real_name,json_object_get_string(value));
			else if(strcmp(key,"email_address")==0)
				strcpy(ui->Mail,json_object_get_string(value));
			else if(strcmp(key,"personal_web_uri")==0)
				strcpy(ui->personal_web_uri,json_object_get_string(value));
			else if(strcmp(key,"birthday")==0)
				strcpy(ui->birthday,json_object_get_string(value));
			else if(strcmp(key,"qq_number")==0)
				strcpy(ui->qq_number,json_object_get_string(value));
			else if(strcmp(key,"sex")==0)
				ui->sex=json_object_get_int(value);
			else if(strcmp(key,"phone_number")==0)
				strcpy(ui->Phone,json_object_get_string(value));
			else if(strcmp(key,"cell_phone_number")==0)
				strcpy(ui->Mobile,json_object_get_string(value));
			else if(strcmp(key,"photo")==0)
				strcpy(ui->Photo,json_object_get_string(value));
			else if(strcmp(key,"main_post")==0)
				strcpy(ui->main_post,json_object_get_string(value));
			else if(strcmp(key,"posts")==0)
				strcpy(ui->posts,json_object_get_string(value));
		}
	}
	json_object_put(pobj);
	return 0;
}

int analysis_token(char* str,Token tok)
{
	json_object *pobj = json_tokener_parse(str);
	//json_object *pobj = json_object_from_file(str);
	if(NULL==pobj)
		return -1;

	json_object_object_foreach(pobj,key,value)
	{
		if(strcmp(key,"access_token")==0)
			strcpy(tok->access_token,json_object_get_string(value));
		else if(strcmp(key,"refresh_token")==0)
			strcpy(tok->refresh_token,json_object_get_string(value));
		else if(strcmp(key,"username")==0)
			strcpy(tok->user_name,json_object_get_string(value));
	}
	json_object_put(pobj);
	return 0;
}

int analysis_error(char* str,char* out)
{
	json_object *pobj = json_tokener_parse(str);
	//json_object *pobj = json_object_from_file(str);
	if(NULL==pobj)
		return -1;
	json_object *obj=NULL;
	if(json_object_object_get_ex(pobj,"error",&obj))
	{
		strcpy(out,json_object_get_string(obj));
	}
	json_object_put(pobj);
	return 0;
}

int analysis_res_error(char* str)
{
	int ret=-10000;
	json_object *pobj = json_tokener_parse(str);
	//json_object *pobj = json_object_from_file(str);
	if(NULL==pobj)
		return ret;
	json_object *obj=NULL;
	if(json_object_object_get_ex(pobj,"ret",&obj))
	{
		ret=json_object_get_int(obj);
	}
	json_object_put(pobj);
	return ret;
}

int analysis_res_notify(char* str,char* out)
{
	int ret=-1;
	json_object *pobj = json_tokener_parse(str);
	if(NULL==pobj)
		return ret;

	char* tmp=NULL;
	json_object *obj=NULL;
	if(json_object_object_get_ex(pobj,"messages",&obj))
	{
		uint msg_len=json_object_array_length(obj);
		if(msg_len>0)
		{
			int i;
			json_object* temp_msg=NULL;
			for(i=0;i<msg_len;i++)
			{
				temp_msg=json_object_array_get_idx(obj,i);
				if(NULL==temp_msg)
				{
					continue;
				}
				int flg=0;
				json_object_object_foreach(temp_msg,key,value)
				{
					if(strcmp(key,"operate_name")==0)
					{
						if(strcmp("iconstate",json_object_get_string(value))==0)
						{
							flg=1;
						}
					}
					if(flg&&(strcmp(key,"data_body")==0))
					{
						tmp=(char*)json_object_get_string(value);
					}
				}
				if(NULL!=tmp)
					break;
			}
		}
	}
	if(NULL!=tmp)
	{
		base64_decode(tmp,out);
		ret=0;
	}
	else
		ret=1;
	json_object_put(pobj);
	return ret;
}
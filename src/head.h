//共用的头文件
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <netinet/tcp.h>
#include <signal.h>
#include "freetdstodb.h"
#include "log.h"
#include "curl_ctl.h"
#include "encode_and_decode.h"
#include "json_analysis.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef LINUX
#include <sys/epoll.h>
#else
#include <poll.h>
#endif

#define DATA_BUF_LEN 1024*4 //中间件收发缓冲区

//XML头
#define MO_XML_HEAD "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

/***************************************业务号码************************************/
#define BUS_SERVER_PUSH 0   //建立服务器推送连接
#define BUS_LOGIN 1 //用户登录
#define BUS_CANCELLATION 2  //用户注销
#define BUS_INDIVIDUAL_SESSION 31   //个人会话
#define BUS_GROUP_SESSION 32    //群会话
#define BUS_MULTIPLAYER_SESSION 33  //多人会话
#define BUS_OL_MOBILE 41    //获取移动端在线成员列表
#define BUS_OL_PC 42    //获取PC端在线成员列表
#define BUS_ORG_STU 5   //获取组织结构
#define BUS_GROUPER 6   //获取群成员
#define BUS_GROUP_SHIELD 7  //群消息屏蔽
#define BUS_GROUP 8 //获取用户所在群的信息
#define BUS_AVATAR 9    //获取用户头像
#define BUS_LOGINER_MSG 10  //获取用户其他信息
#define BUS_MULTIPLAYER 11  //获取讨论组成员
#define BUS_MULTI 12    //获取用户所在讨论组的信息
#define BUS_NEW_GROUP 13    //新建群
#define BUS_NEW_MULTIPLAYER_SESSION 14  //新建多人会话
#define BUS_ADD_MULTIPLAYER_SESSION 15  //添加多人会话成员
#define BUS_EXIT_GROUP 16   //退出群
#define BUS_EXIT_MULTI 17   //退出多人会话
#define BUS_UPDATE_LOGINER_MSG 18   //修改个人资料
#define BUS_CON_PIC 19  //获取会话内容中的图片
#define BUS_CHE_PHOTO 20	//验证用户头像
#define BUS_CHE_NOT 21	//获取通知公告未读数
#define BUS_CON_SUD 22	//获取会话内容中的语音文件
//#define BUS_CIMS_ID 21	//获取CIMS的用户ID
/***********************************************************************************/

enum DEV_TYPE
{
	IGNORE=0x0,
	ANDROID,
	IOS,
	WP,
};

enum GroupMutil
{
    CTRLALL=0,
    CTRLPERSON,
    CTRLGROUP,
    CTRLMUTIL,
};

enum TRANSTYPE
{
    SYN = 0x0,
    ACK = 0x1,
};

enum TRANSRESULT
{
    RET_SUCCEED = 0x0,
    RET_FAILD = 0x1,
    RET_REFUSE = 0x2,
};

// 应用层命令状态：
enum BusinessState  //unsigned char
{
    REQUEST,    // 请求
    RESPONSE,   // 响应

    RECORDLINK, // 记录最近联系人

    IDORPASSWORD,   // 登录时，ID或者密码错误
    BS_NOBOUND, // 挂载登录时，未绑定原生账号

    FAILD,  // 失败
    NOSUPPORT,  // 不支持此命令

    REFUSE, // 拒绝，目前用于拒绝入群邀请
};

// 加密类型：
enum EncryptType    //unsigned char
{
    NONE,   // 不加密
    RSA128,
    RSA256,
    DES24,  // 3DES加密
};

// 中间件业务号
enum MiddleCommand  //short
{
    MC_EXTAPP_CONN, // 扩展应用连接至中间件
    MC_EXTAPP_SCHEMA,   // 扩展应用将自己的在线列表交给中间件
    MC_EXTAPP_GETONLINELIST,    // 扩展应用从中间件获取指定服务的在线列表

    MC_BTPC_LOGIN = 0x100,  // 蜜聊PC端登录
    MC_BTPC_LOGOUT,         // 蜜聊PC端登出
    MC_BTPC_CTM_BASEINFO,   // 用户修改自身基本资料
    MC_BTPC_CTM_PHOTO,      // 用户修改自身照片
    MC_BTPC_CTM_MOOD,       // 用户更新心情
    MC_BTPC_PTOP_MSG,       // 个人点对点通信信息
    MC_BTPC_GROUP_CREATE,   // 群组创建
    MC_BTPC_GROUP_DELETE,   // 群组解散
    MC_BTPC_GROUP_ADDUSER,  // 群组加人
    MC_BTPC_GROUP_DELUSER,  // 群组删人
    MC_BTPC_GROUP_NOTICE,   // 群公告修改
    MC_BTPC_GROUP_NOTIFY,   // 用户修改群组通知设置
    MC_BTPC_GROUP_MSG,      // 群消息
    MC_BTPC_MULTI_CREATE,   // 讨论组创建
    MC_BTPC_MULTI_ADDUSER,  // 讨论组加人
    MC_BTPC_MULTI_DELUSER,  // 讨论组删人
    MC_BTPC_MULTI_SUBJECT,  // 修改讨论组主题
    MC_BTPC_MULTI_MSG,      // 讨论组消息

    MC_BTANDRIOD_LOGIN = 0x200, // 蜜聊安卓端登录
    MC_BTANDRIOD_LOGOUT,        // 蜜聊安卓端登出
    MC_BTANDRIOD_CTM_BASEINFO,  // 用户修改自身基本资料
    MC_BTANDRIOD_CTM_PHOTO,     // 用户修改自身照片
    MC_BTANDRIOD_CTM_MOOD,      // 用户更新心情
    MC_BTANDRIOD_PTOP_MSG,      // 个人点对点通信信息
    MC_BTANDRIOD_GROUP_CREATE,  // 群组创建
    MC_BTANDRIOD_GROUP_DELETE,  // 群组解散
    MC_BTANDRIOD_GROUP_ADDUSER, // 群组加人
    MC_BTANDRIOD_GROUP_DELUSER, // 群组删人
    MC_BTANDRIOD_GROUP_NOTICE,  // 群公告修改
    MC_BTANDRIOD_GROUP_NOTIFY,  // 用户修改群组通知设置
    MC_BTANDRIOD_GROUP_MSG,     // 群消息
    MC_BTANDRIOD_MULTI_CREATE,  // 讨论组创建
    MC_BTANDRIOD_MULTI_ADDUSER, // 讨论组加人
    MC_BTANDRIOD_MULTI_DELUSER, // 讨论组删人
    MC_BTANDRIOD_MULTI_SUBJECT, // 修改讨论组主题
    MC_BTANDRIOD_MULTI_MSG,     // 讨论组消息

    MC_BTANDRIOD_PTOP_IMAGE = 0x300,        // 个人点对点图片信息
    MC_BTANDRIOD_GROUP_IMAGE,
    MC_BTANDRIOD_MULTI_IMAGE,
};

enum ExtAppType
{
    EAT_NOTREG, // 未知的服务类型
    EAT_BTPCINSTANT,    // 蜜聊PC版即时通讯服务
    EAT_BTANDRIOD,  // 蜜聊安卓版服务
};

enum GroupNotifyType
{
    GNT_RECVANDPOPUP,   //接收并弹出
    GNT_RECVNOTPOPUP,   //接收不提醒
    GNT_REFUSE, //屏蔽
};

/***********************************中间件交互*******************************************/


typedef struct  // 业务包包头
{
    uint16_t Command; // 业务号
    uint8_t Result;   // 业务执行结果
    uint8_t Enctype;  // 数据域的加密类型
    uint32_t Datalen;   // 数据域实际长度
    uint32_t Srclen;    // 数据域密文的源数据的长度，若没有加密则应与Datalen等值
} BsnsPacket,*BsPt;

typedef struct  //传输包包头
{
    union
    {
        uint16_t Type:4;  // 包类型，传输包/响应包
        uint16_t Ret:4;   //
        uint16_t Window:8;// 发送/接收窗口
        uint16_t Flag;
    } State;
    uint16_t Count;   // 分包总数
    uint16_t Index;   // 分包索引，从0开始
    uint16_t Datalen;// 数据域长度
    uint16_t DataVerfy;   // 数据域CRC
    uint16_t HeadVerfy;   // 包头CRC
} TransPacket,*TsPt;

//各种业务包
typedef struct
{
    BsnsPacket bp;
    char id[48];
    char context[0];
} MS32CHARINFO,*MS32_CHARINFO;

//1)用户登录，id为登录用户ID
//2）用户注销，id为注销用户ID
//3）更新用户个人资料，id为用户ID

typedef struct
{
    BsnsPacket bp;
	//uint32_t sys_type;
    char srcid[48];
    char desid[48];
    char context[0];
} MS64CHARINFO,*MS64_CHARINFO;

typedef struct
{
    char srcid[48];
    char desid[48];
    char context[0];
} RMS64CHARINFO,*RMS64_CHARINFO;


//1）个人会话，srcid为发送人ID，desid为目标人ID，context为会话内容,注意，会话内容为经过URL编码的UTF-8字符串

typedef struct
{
    BsnsPacket bp;
    uint32_t ntype;
    char context[0];
} MSEXTAPPREG_REQ,*MSEXTAPP_REG_REQ;

//1）服务端登录，ntype为EAT_BTANDRIOD
//2）服务端发送自己的在线用户列表,context为用户列表XML串，当没有用户时，它只有XML头
//3）服务端向中间件请求PC服务端在线列表XML
//4) 新群/讨论组创建和删除,ntype为新群/讨论组ID（PC与移动相同）
//5）讨论组加人，ntype为讨论组ID，context为新添加的用户ID，格式为:用户ID|，如:用户AID|用户BID|用户CID|

//6）中间件向服务端发送PC端的在线列表XML

typedef struct
{
    BsnsPacket bp;
    char uid[48];
    uint32_t gid;
    uint32_t opertype;
    char context[0];
} MSGROUPCTRL,*MS_GROUP_CTRL;

typedef struct
{
    char uid[48];
    uint32_t gid;
    uint32_t opertype;
    char context[0];
} RMSGROUPCTRL,*RMS_GROUP_CTRL;


//1)屏蔽群消息，uid用户ID，gid为群号，opertype为屏蔽/解屏蔽

typedef struct
{
    BsnsPacket bp;
	//uint32_t sys_type;
    char uid[48];
    uint32_t gid;
    char context[0];
} MSGROUP,*MS_GROUP;

typedef struct
{
    char uid[48];
    uint32_t gid;
    char context[0];
} RMSGROUP,*RMS_GROUP;


//1）退出群/讨论组，uid为用户ID，gid为群/讨论组ID
//2）讨论组/群组会话，uid为会话用户ID，gid为讨论组/群组ID，context为会话内容，注意，会话内容为经过URL编码的UTF-8字符串

//3）中间件提醒服务端，群/讨论组消息，gid为群/讨论组ID，uid为发送人ID

/*typedef struct
{
    uint32_t ntype;
    uint32_t len;
    char context[0];
} REXML,*RE_XML;*/

/***************************************************************************************/


/*******************************************CGI交互*************************************/
typedef struct  //CGI发来的消息
{
    uint32_t packet_len;    //包长
    uint32_t type;  //业务号
    char sender[48];    //用户id
    char recver[48];    //目标id，用于对话
	uint32_t dev_type;	//设备系统类型
    uint32_t len;   //context的长度
    char context[0];    //业务信息
} CGI_MSG,*CM;

typedef struct  //回应给CGI的信息
{
    uint32_t len;   //内容长度
    char context[0];    //内容文本
} CGI_RM_MSG,*CRM;

/*typedef struct Depart
{
    int did;
    char dpm[128];
    struct Depart *next;
} subdpm,*subdpmt;*/

struct _DPMNODE	// 部门节点信息
{
	int DpmId;
	char DpmName[192];
};

/***************************************************************************************/

typedef struct imf_list	//通知队列节点
{
    struct imf_list *next;
    uint len;
    char* context[0];
} IMF_LIST,*IL;

typedef struct user_link    //用户在线结构体，用于组成链表
{
    char id[48];    //用户id
	char access_token[36];	//访问令牌
	char refresh_token[36];	//更新令牌
	UserInfo usrinfo;	//用户信息
	//char Nick_name[48];	//昵称
    int fd; //CGI域套接字文件描述符
    //pthread_t time; //计时线程(8byte)
    int flag;	//使用中标识
    IL il;	//通知消息队列的头指针
    struct user_link *next;
} User_Linking,*UL;

/*************************************全局变量******************************************/

pthread_mutex_t mutex,mutex_cgi; //线程锁，防止多个线程同时发送消息给中间件和CGI
//pthread_mutex_t mutex_sql;  //防止多个线程同时读写数据库
int mi_fd;  //中间件套接字
int cgi_fd; //CGI域套接字服务端文件描述符
UL user;    //用户在线链表
sem_t mi_send_recv_ctrl;    //中间件启动连接控制
pthread_t rad_thread;	//中间件接收线程

char* org_stu;  //组织结构
//char* tmp_stu;  //临时组织结构缓存区

int g_nOrgStuLen;	//组织结构缓冲区长度

char *PC_OL;    //PC端在线列表
char *MO_OL;    //移动端在线列表

//数据库操作句柄
//DBPROCESS *dbprocess;

//数据库连接
char* DBSERVER;  //数据库服务器连接地址
char* SQL_DBNAME;  //数据库名
uint MAX_DATA;	//可以获取数据库的最大单条数据
char* SQL_USER;   //用户名
char* SQL_PASSWD; //密码
uint SQL_LINK_COUNT;	//数据库连接池

//CGI通信
int MAX_CGI_LINK;   //CGI最大连接数
char* UNIX_PATH;    //域套接字连接符路径(用户必须设置有读写权限的目录内，套接口的名称必须符合文件名命名规则且不能有后缀，该变量和CGI头文件中的同名宏必须相同)
char* LOG_PATH;	//日志路径

//中间件通信
int MI_PORT;    //中间件端口
char* MI_ADDR; //中间件IP

//int CONPANY_ID;    //公司ID

//用户中心通信
char* CLIENT_ID;
char* CLIENT_PASSWORD;
char* CLIENT_KEY;
char* USER_ADDR;
char* LISTEN_MSG_URL;

int fresh_org;	//是否完成组织结构更新标识

char clientkey[80];	//保存解码后的CLIENT_KEY
uint keylen;	//解码后的CLIENT_KEY长度
char author[64];	//存储OAuth验证码

//加密密钥
static unsigned char deskey[24] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+' };

/***************************************************************************************/
/*************************************************main.c**********************************/
void exitbt(int signo);	//关闭函数，用于擦屁股

/***********************************************net.c***********************************/
int Init_Mi_TCP(char* ip,int port); //初始化连接中间件
int Exit_Mi_TCP(); //销毁中间件连接
void* MI_Read(void* args);  //中间件接收线程
int MI_Write(const char *bsnsdata, int len,int flag);    //与中间件交互的业务包发送函数
void HandleBusiness(BsPt pBsns, const char *data, uint len);  // 与中间件交互的业务数据处理函数
void* ReLink_mi(void* arg);	//重连中间件

/*****************************************************************************************/

/**************************************protocol.c*****************************************/

TsPt calcULate_verfy(TsPt tp,unsigned char *data, unsigned short len);  //初始化传输包
BsnsPacket BsnsPacket_init(unsigned short cmd, unsigned char ret, unsigned char enctype,int len);   //初始化业务包

/*****************************************************************************************/

/*****************************************cgi_ctrl.c*****************************************/

void setnonblocking(int sock);  //设定非阻塞套接字
void* Flush_CGI(void* arg); //保活线程
void* CGI_Link(void* nfd);  //CGI会话连接
int Zero_RE(int fd,char* context,int len,int type);  //0业务的回发，返回值为-1时失败，其它值则为成功
void* Check_OutLine(void* nfd); //掉线判断
CRM Business_deal(CM cm,int fd);    //CGI业务处理
void cgi_all_send(char* msg,char* me);   //客户端广播通知

/*****************************************************************************************/

/*****************************************cgi_business.c**********************************/

char* Check_Logined(char* loginer); //检查登录
char* Login(char* loginer,char* password);  //登录
char* Cancellation(char* loginer);   //注销
char* GetOrg_Stu(); //获取组织结构
char* GetPc_Ol();   //获取PC端用户在线列表
char* GetMo_Ol();   //获取移动端用户在线列表
char* ServerPush(char* loginer,char* msg,int fd,UL ul);   //服务器接收预备推送
char* GetGrouper(char* gid);    //获取群成员列表
char* SetShield(char* loginer,char* context);   //设置群消息屏蔽
char* GetGroup(char* loginer);  //获取群列表
char* GetAvatar(char* loginer); //获取头像
char* GetOrger_Msg(char* loginer,char* orger);    //获取个人资料
char* GetMultiplayer(char* mid);    //获取讨论组成员列表
char* GetMulti(char* loginer);  //获取讨论组列表
char* NewGroup(char* loginer,char* context);    //新建群
char* NewMulti(char* loginer,char* context);    //新建讨论组
char* AddMulti(char* context);  //添加讨论组成员
char* ExitGroup(char* loginer,char* gid);   //退出群
char* ExitMulti(char* loginer,char* mid);   //退出多人会话
char* UpdateLoginerMsg(char* loginer,char* context);    //修改本人个人资料
char* GetPicture(char* pid);    //获取对话内容中的图片
char* Talk(int type,char* src,char* des,char* context,uint32_t len,uint32_t systype);    //发送会话
char* GetImf(UL ul,char* loginer); //获取通知队列
//void strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl); //字符串替换
//int Count(char *const a,char *const b); //子字符串统计
char* Check_Photo(char* context);	//验证头像
char* Check_fresh_notify(char* loginer);	//获取当前通知公告未读数
//char* Get_CIMS_ID(char* uid);	//获取CIMS的ID
char* GetSound(char* sid);    //获取对话内容中的语音文件
/***************************************************************************************/

/****************************************mi_business.c************************************/

void OnGetOnlineList(unsigned char Result, char* srcdata, int srclen);  //获取PC端在线列表
int MSG_RECV(char* srcdata,int srclen,int type);   //获取PC端消息
void MSG_INFO(int type);   //获取PC端通知
//char Char2Int(char ch); //字符转整型
//char Str2Bin(char *str);    //字符转二进制
//unsigned char* UrlDecode(char* str);    //URL解码
int Link_Mi();  //连接中间件确认并发送自己的在线成员列表
int SEND_GET_PC_ONLINE_LIST();  //获取PC端在线成员列表
void OnMiddleLogin(unsigned char result);   //程序登录中间件
void OnMiddleSchema(unsigned char result);  //向中间件发送请求,请求PC在线列表
int Send_MO_OL();   //发送移动端在线列表
void* CHECK_MI_LINK(void* arg);	//检查中间件连接状态线程

/***************************************************************************************/

/******************************************************dbctrl.c******************************************************/

int check_up(char *uid,char *pass); //确认用户已经在线
int set_group(char *uid,int gid,int type);  //设置群屏蔽
int insert_group(char* uid,char* name,char* theme,char* id,int* gid,int type);  //新增群/讨论组
int insert_mutil_orger(int gid,char* ids);  //新增讨论组成员
int update_org_info(char* uid,char* Phone,char* Mobile,char* Mail,char* Mood);  //修改成员信息
int exit_group_mutil(char* uid,int gid,int type);   //退出群/讨论组
int delete_msg(char* ids,int type); //删除离线消息
int insert_talklist(char* src,char* des,char* context,uint32_t len,int type,uint32_t systype);   //新增会话消息
int check_user_group_or_mutil(int gid,char* uid,int type);  //确认用户在某个群/讨论组中
//int get_org_stu(int dpmid); //获取组织结构
//int get_subdepartmemt_user(char* departmemt,int did);   //获取单个部门组织结构
char* get_group_or_mutil(char* id,int type);    //获取群/讨论组
char* get_member(int gid,int type); //获取群/讨论组成员
char* get_info(char* uid);  //获取个人信息
char* get_photo(char* uid); //获取头像二进制
char* search_info(char* loginer,int type);  //获取离线消息
char* get_picture(int pid,char* ext);   //获取对话图片
int get_group_mutil_user(char* uid,int gid,int type);   //获取群/多人会话成员并发送通知
int check_user_photo(char* uid,char* md5);	//验证数据库中photo的MD5字段
//获取组织结构
//int GetOnlineCtms(int dpmid);
//int GetSubDpmsAndWorkersForCvst(int dmpid);
//char* getCIMS_id(char *uid);	//查询CIMS的ID
int get_sign(char* uid,char* sign);	//获取用户签名
int update_user_mood(char* uid,char* Mood);	//更新用户签名
char* get_sound(int sid,char* ext);   //获取对话语音文件
/********************************************************************************************************************/

/********************************************userlink.c*********************/

UL get_point(UL head,char* loginer);    //获取在线用户节点
int get_len(UL head);   //获取在线用户个数
UL insert_point(UL head,char* loginer,int fd); //添加在线用户节点（登录）
int delete_point_log(UL head,UL uid);    //删除在线用户节点
int update_point_fd(UL head,char* loginer,int fd);  //刷新在线用户连接文件描述符
//int update_point_pth(UL head,int fd,pthread_t time);    //修改用户在线链表节点的线程描述符
char* NULL_OL();    //PC端没有用户在线
UL get_point_fd(UL head,int fd);//获取用户在线链表节点
void DeleteList(IL head);   //删除通知队列
int insert_imf(IL head,char* context,uint len);    //新增通知
int delete_imf(IL head,IL uimf);    //删除通知
IL get_imf(IL head);    //获取通知
void flush_list(UL head);   //获取当前在线用户列表
void DeleteULList(UL head); //清空用户在线列表

/***************************************************************************************/

/***********************************************webctrl.c**********************************************/
int setAuth(char* client_id,char* client_pass);	//创建OAuth
int web_check_up(char *uid,char *pass,Token retn);	//用户中心登录验证
char* web_get_schema(UL ul,int flag);	//获取组织结构
char* web_get_info(UL ul,char* desid,int flag);	//获取用户信息
int web_check_avatar(char* pid,char* md5val);	//验证头像文件MD5
int web_updata_info(UL ul,char* Mood,char* Other);	//更新用户信息
char* web_get_notify(UL ul,int flag);	//获取通知公告未读数
void fresh_schema();	//更新组织结构
void* listen_schema();	//组织结构更新通知接收线程
/******************************************************************************************************/

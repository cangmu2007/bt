//�м���շ�,mi_fdΪȫ�ֱ���,�м�����ļ�������

#include "head.h"

int Init_Mi_TCP(char* ip,int port)
{
	if((mi_fd = socket(AF_INET,SOCK_STREAM,0))<0) //TCP��socket
    {
        perror("TCP mi socket");
		writelog("TCP mi socket");
        return -1;
    }

	//�������ӳ�ʱ
	struct timeval timeout = {10,0}; 
	setsockopt(mi_fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));

	//������
	int keepIdle = 1000;
	int keepInterval = 30;
	int keepCount = 10;
	setsockopt(mi_fd,IPPROTO_TCP, 4,(void*)&keepIdle,sizeof(keepIdle));
	setsockopt(mi_fd,IPPROTO_TCP,5,(void*)&keepInterval,sizeof(keepInterval));
	setsockopt(mi_fd,IPPROTO_TCP, 6,(void*)&keepCount,sizeof(keepCount));

	struct sockaddr_in MI_addr;
    memset(&MI_addr, 0,sizeof(MI_addr));    //TCP���ӵ�ַ
    MI_addr.sin_family = AF_INET;
    MI_addr.sin_addr.s_addr = inet_addr(ip);
    MI_addr.sin_port = htons(port);

    if(connect(mi_fd, (struct sockaddr *)&(MI_addr), sizeof(MI_addr))<0) //�����м��
    {
        perror("TCP mi connect");
		writelog("TCP mi connect");
		close(mi_fd);
		mi_fd=-1;
        return -1;
    }

    /*if(pthread_mutex_init(&mutex_ol,NULL)<0)    //��ʼ��������
    {
        printf("pthread_mutex_init ol error\n");
		writelog("pthread_mutex_init ol error");
        return -1;
    }*/

	if(pthread_mutex_init(&mutex,NULL)<0) //��ʼ��������
    {
        printf("pthread_mutex_init mi error\n");
		writelog("pthread_mutex_init mi error");
        return -1;
    }

    if(sem_init(&mi_send_recv_ctrl, 0, 1)<0)
    {
        perror("sem_init");
		writelog("sem_init");
        return -1;
    }

	/*if((mi_send_recv_ctrl=sem_open("btsem", O_CREAT, 0666, 1))==SEM_FAILED)
    {
        perror("sem_open");
		writelog("sem_open");
        return -1;
    }*/

    if(pthread_create(&rad_thread, NULL, (void*)MI_Read, NULL)<0)   //���������߳�
    {
        printf("pthread_create recv\n");
		writelog("pthread_create recv\n");
        return -1;
    }
    usleep(500);

    if(Link_Mi()<0)
    {
        printf("Failed to connect Middleware\n");
		writelog("Failed to connect Middleware");
        return -1;
    }
    sem_wait(&mi_send_recv_ctrl);

    if(Send_MO_OL()<0)
    {
        printf("Failed to send MO_OL to MI\n");
		writelog("Failed to send MO_OL to MI");
		return -1;
    }
    sem_wait(&mi_send_recv_ctrl);

    if(SEND_GET_PC_ONLINE_LIST()<0)
    {
        printf("Failed to Requested PC_OL\n");
		writelog("Failed to Requested PC_OL");
		return -1;
    }
	return 0;
}

int Exit_Mi_TCP()
{

	if(pthread_cancel(rad_thread)<0)  //�ر��м�����߳�
    {
        printf("pthread_cancle recv\n");
		writelog("pthread_cancle recv");
        return -1;
    }
    if(pthread_join(rad_thread,NULL)<0) //�������߳�
    {
        printf("pthread_join recv\n");
		writelog("pthread_cancle recv");
        return -1;
    }

    /*if(pthread_mutex_destroy(&mutex_ol)<0)  //���ٻ�����
    {
        printf("pthread_mutex_destroy(ol)\n");
		writelog("pthread_mutex_destroy(ol)");
        return -1;
    }*/

    if(pthread_mutex_destroy(&mutex)<0) //���ٷ��Ͷ˻�����
    {
        printf("pthread_mutex_destroy(mi)\n");
		writelog("pthread_mutex_destroy(mi)");
        return -1;
    }

    if(sem_destroy(&mi_send_recv_ctrl)<0)
    {
        perror("sem_destroy");
		writelog("sem_destroy");
        return -1;
    }

	/*if(sem_close(mi_send_recv_ctrl)<0)
	{
		perror("sem_close");
		writelog("sem_close");
        return -1;
	}

	sem_unlink("btsem");*/

	if(mi_fd!=-1)
	{
		close(mi_fd);   //�ر��м���׽ӿ�
		mi_fd=-1;
	}

	return 0;
}

int MI_Write(const char *bsnsdata, int len,int flag)
{
	int num = -1;
	if(mi_fd==-1)
		return -1;   
    unsigned short max_len = DATA_BUF_LEN;
    int headlen = sizeof(TransPacket);
    unsigned short nCutLen = max_len - headlen;// �ְ���������󳤶�
    unsigned short nCount = (0 == len%nCutLen) ? (len/nCutLen) : (len/nCutLen + 1);// �ְ�����
    unsigned short nCurlen = 0;// ��ǰ�ְ����ݳ���
    unsigned char *pOffset = NULL;// ��ǰ����λ��
    unsigned short i = 0;
    char temp[DATA_BUF_LEN]= {0};
    memset(temp,0,DATA_BUF_LEN);
    TsPt transpacket=(TsPt)malloc(sizeof(TransPacket));
	memset(transpacket,0,sizeof(TransPacket));
    if(NULL==transpacket)
    {
        perror("malloc error");
		writelog("malloc error");
        return -1;
    }
    transpacket->Count = nCount;
    pthread_mutex_lock(&mutex); //����
    for(i = 0; i < nCount; i++)
    {
        // �������ķְ��Ļ���ǰ�и��Ϊ�и��������Ϊ���ʣ����
        nCurlen = (i != nCount - 1) ? nCutLen : (len - i*nCutLen);
        pOffset = (unsigned char *)(bsnsdata + i*nCutLen);// Ų������ƫ��ָ��
        transpacket->Index = i;// ���÷ְ�����
        transpacket=calcULate_verfy(transpacket, pOffset, nCurlen); // ���������򡢳��ȼ�������У�顢��ͷУ��

        // ��ͷ���������뻺����
        memcpy(temp,  transpacket, headlen);
        memcpy(temp + headlen, pOffset, nCurlen);

        // �ֵ�һ���ͷ�һ��
        int total = nCurlen + headlen;
        int nSend = 0;
        while(nSend < total)
        {
            num = send(mi_fd, temp + nSend, total - nSend, MSG_NOSIGNAL);
            if(-1 == num)
            {
				perror("TCP send");
				writelog("TCP send");
                break;
            }
            else
            {
                nSend += num;
            }
        }
        if(nSend != total)
        {
            break;
        }
    }
	free(transpacket);
	pthread_mutex_unlock(&mutex);   //����
	if(num==-1&&flag==1)
	{
		Exit_Mi_TCP();
		sleep(3);
		Init_Mi_TCP(MI_ADDR,MI_PORT);
		num=MI_Write(bsnsdata,len,0);
	}
    return (num == -1) ? -1 : len + headlen;
}

void* MI_Read(void* args)   //TCP���ղ�����Ӧ����
{
    //pthread_detach(pthread_self());   //�����߳�
    int num = -1;
    char rbuf[DATA_BUF_LEN]= {0};

    int ret = 0,head;
    uint nHeadLen = sizeof(TransPacket);
    char *pBsns = 0;    // ����ҵ�������
    uint nBsnsLen = 0;   // ����ҵ������ݳ���

    for(;;)
    {
        memset(rbuf,0,DATA_BUF_LEN);    //��ջ�����

        // 1������12�ֽڰ�ͷ
        head = 0;
        while(head < nHeadLen)
        {
            num = recv(mi_fd, rbuf + head, nHeadLen - head, 0);
            if(num <= 0)
            {
                perror("TCP recv");
				writelog("TCP recv");
                ret = -1;
                break;
            }
            else
            {
                head += num;
            }
        }
        if(-1 == ret)
        {
            break;
        }
        // 2���������ͷָʾ��������ҵ�����ݵĳ��ȶ����������
        TsPt pTrans = (TsPt)rbuf;
        uint datalen = 0;
        char *data = rbuf + nHeadLen;
        while(datalen < pTrans->Datalen)
        {
            num = recv(mi_fd, data + datalen, pTrans->Datalen - datalen, 0);
            if(num <= 0)
            {
                perror("TCP recv");
				writelog("TCP recv");
                ret = -1;
                break;
            }
            else
            {
                datalen += num;
            }
        }
        if(-1 == ret)
        {
            break;
        }
        // 3������һ�������Ĵ����֮����ݴ����ͷ��ָʾ���зְ�����
		char* temp = (char*)realloc(pBsns, nBsnsLen + pTrans->Datalen);
		if(!temp)
		{
			ret=-1;
			break;
		}
        pBsns=temp;
        memcpy(pBsns + nBsnsLen, data, pTrans->Datalen);
        nBsnsLen += pTrans->Datalen;
        if(pTrans->Count == pTrans->Index + 1)
        {
            // ��ҵ������д���
            HandleBusiness((BsPt)pBsns, (const char*)(pBsns + sizeof(BsnsPacket)), nBsnsLen - sizeof(BsnsPacket));
            free(pBsns);
            pBsns = NULL;
            nBsnsLen = 0;
        }
    }
    if(pBsns != 0)
    {
        free(pBsns);
		pBsns=NULL;
    }
    if(-1==ret)
    {
		pthread_t tid;
		if(pthread_create(&tid,NULL,(void*)ReLink_mi,NULL)<0)
		{
			printf("ReLink_mi error\n");
			writelog("ReLink_mi error");
		}
        pthread_exit((void*)&ret);  //����ر��̲߳�����-1
    }
	return NULL;
}

void* ReLink_mi(void* arg)
{
	pthread_detach(pthread_self()); //�����߳�
	Exit_Mi_TCP();
	for(;;)
	{
		sleep(3);
		if(Init_Mi_TCP(MI_ADDR,MI_PORT)<0)
		{
			if(mi_fd!=-1)
			{
				close(mi_fd);
				mi_fd=-1;
			}
			else
			{
				Exit_Mi_TCP();
			}
		}
		else
			break;
	}
	return NULL;
}

void HandleBusiness(BsPt pBsns, const char *data, uint len)
{
    uint srclen = pBsns->Srclen;
    char *srcdata = NULL;
    switch (pBsns->Command)
    {
        case MC_EXTAPP_CONN:
            OnMiddleLogin(pBsns->Result);
            break;
        case MC_EXTAPP_SCHEMA:
            OnMiddleSchema(pBsns->Result);
            break;
        case MC_EXTAPP_GETONLINELIST:
            srcdata=(char*)malloc(srclen+1);
			if(NULL==srcdata)
			{
				break;
			}
			memset(srcdata,0,srclen+1);
            memcpy(srcdata,data,srclen);
            OnGetOnlineList(pBsns->Result, srcdata, srclen);
            free(srcdata);
            break;
        case MC_BTPC_LOGIN:
            if(SEND_GET_PC_ONLINE_LIST()<0)
			{
                printf("Failed to Requested PC_OL\n");
				writelog("Failed to Requested PC_OL");
			}
            break;
        case MC_BTPC_LOGOUT:
            if(SEND_GET_PC_ONLINE_LIST()<0)
			{
                printf("Failed to Requested PC_OL\n");
				writelog("Failed to Requested PC_OL");
			}
            break;
        case MC_BTPC_CTM_BASEINFO:
            MSG_INFO(CTRLPERSON);
            break;
        case MC_BTPC_CTM_MOOD:
            break;
        case MC_BTPC_GROUP_NOTICE:
            break;
        case MC_BTPC_PTOP_MSG:
			srcdata=(char*)malloc(srclen+1);
			if(NULL==srcdata)
			{
				break;
			}
			memset(srcdata,0,srclen+1);
            memcpy(srcdata,data,srclen);
            MSG_RECV(srcdata, srclen,CTRLPERSON);
            free(srcdata);
            break;
        case MC_BTPC_GROUP_MSG:
            srcdata=(char*)malloc(srclen+1);
			if(NULL==srcdata)
			{
				break;
			}
			memset(srcdata,0,srclen+1);
            memcpy(srcdata,data,srclen);
            MSG_RECV(srcdata, srclen,CTRLGROUP);
            free(srcdata);
            break;
        case MC_BTPC_MULTI_MSG:
            srcdata=(char*)malloc(srclen+1);
			if(NULL==srcdata)
			{
				break;
			}
			memset(srcdata,0,srclen+1);
            memcpy(srcdata,data,srclen);
            MSG_RECV(srcdata, srclen,CTRLMUTIL);
            free(srcdata);
            break;
        case MC_BTPC_GROUP_CREATE:
            MSG_INFO(CTRLGROUP);
            break;
		case MC_BTPC_MULTI_CREATE:
			MSG_INFO(CTRLMUTIL);
            break;
        case MC_BTPC_GROUP_DELETE:
            MSG_INFO(CTRLGROUP);
            break;
        case MC_BTPC_MULTI_DELUSER:
            MSG_INFO(CTRLMUTIL);
            break;
		case MC_BTPC_MULTI_SUBJECT:
            MSG_INFO(CTRLMUTIL);
            break;
        default:
            break;
    }
}

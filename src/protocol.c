//BsnsPacket,TransPacket�ĸ��ֺ�������

#include "head.h"

TsPt calcULate_verfy(TsPt tp,unsigned char *data, unsigned short len)   //�������ʼ��
{
    tp->Datalen = len;
    //tp->DataVerfy = abcsp_crc_block(data, len);
    //tp->HeadVerfy = abcsp_crc_block((unsigned char *)&(tp->State), sizeof(TransPacket) - 2);
    return tp;
}

BsnsPacket BsnsPacket_init(unsigned short cmd, unsigned char ret, unsigned char enctype,int len)    //ҵ�����ʼ��
{
    BsnsPacket bp= {0};
    bp.Command = cmd;
    bp.Result = ret;
    bp.Enctype = enctype;
    bp.Datalen = bp.Srclen=len;
    return bp;
}

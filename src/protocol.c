//BsnsPacket,TransPacket的各种函数处理

#include "head.h"

TsPt calcULate_verfy(TsPt tp,unsigned char *data, unsigned short len)   //传输包初始化
{
    tp->Datalen = len;
    //tp->DataVerfy = abcsp_crc_block(data, len);
    //tp->HeadVerfy = abcsp_crc_block((unsigned char *)&(tp->State), sizeof(TransPacket) - 2);
    return tp;
}

BsnsPacket BsnsPacket_init(unsigned short cmd, unsigned char ret, unsigned char enctype,int len)    //业务包初始化
{
    BsnsPacket bp= {0};
    bp.Command = cmd;
    bp.Result = ret;
    bp.Enctype = enctype;
    bp.Datalen = bp.Srclen=len;
    return bp;
}

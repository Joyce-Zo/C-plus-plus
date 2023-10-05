#include"protocol.h"


PDU *CreatePDU(ALL_SIZE real_msg_size)
{
    ALL_SIZE all_PDU_size = sizeof(PDU)+real_msg_size;
    PDU *pdu = (PDU*)malloc(all_PDU_size); //动态控件申请
    if(NULL==pdu)    exit(EXIT_FAILURE); //如果创建失败，直接退出
    memset(pdu,0,all_PDU_size);
    pdu->all_PDU_size = all_PDU_size;
    pdu->real_msg_size = real_msg_size;
    return pdu;
}

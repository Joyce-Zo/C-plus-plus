#include "online.h"
#include "ui_online.h"
#include"tcpclient.h"
Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::ShowUser(PDU *pdu)
{
    if(pdu == NULL) return ;
    ui->listWidget->clear();
    uint ui_size = pdu->real_msg_size/32;
    char ca_tmp[32];
    for(uint i=0;i<ui_size;i++)
    {
        memcpy(ca_tmp,(char*)(pdu->real_msg_data)+i*32,32);
        ui->listWidget->addItem(ca_tmp);
    }
}

void Online::on_pushButton_add_clicked()
{
    QListWidgetItem* p_item = ui->listWidget->currentItem();
    QString str_his_user_name = p_item->text();
    QString str_login_name = TcpClient::GetInstance().LoginName();

    PDU* pdu = CreatePDU(0);
    pdu->msg_type = ENUM_MSG_TYPE_ADD_REQUEST;
    memcpy(pdu->caData,str_his_user_name.toStdString().c_str(),str_his_user_name.size());
    memcpy(pdu->caData+32,str_login_name.toStdString().c_str(),str_login_name.size());
    TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
    free(pdu);
    pdu = NULL;
}


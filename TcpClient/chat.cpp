#include "chat.h"
#include "ui_chat.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QMessageBox>
#include<QString>
#include<QStringList>
#include<cstring>

Chat::Chat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
}

Chat::~Chat()
{
    delete ui;
}

Chat &Chat::GetInstance()
{
    static Chat instance;
    return instance;
}

void Chat::SetChatName(QString str_name)
{
    m_str_his_name = str_name;
    m_str_my_name = TcpClient::GetInstance().LoginName();
}

void Chat::updateMsg(const PDU *pdu)
{
    if(pdu == NULL) return;
    char ca_my_name[32] = {'\0'};
    memcpy(ca_my_name,pdu->caData,32);
    QString str_msg = QString("%1 say: %2").arg(ca_my_name).arg((char*)(pdu->real_msg_data));
    ui->show_msg->append(str_msg);
}

void Chat::on_pushButton_send_msg_clicked()
{
    QString str_msg = ui->input_meg->text();
    ui->input_meg->clear();
    if(!str_msg.isEmpty())
    {
        PDU *pdu = CreatePDU(str_msg.size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_CHAT_REQUEST;
        memcpy(pdu->caData,m_str_my_name.toStdString().c_str(),m_str_my_name.size());
        memcpy(pdu->caData+32,m_str_his_name.toStdString().c_str(),m_str_his_name.size());

        strcpy((char*)(pdu->real_msg_data),str_msg.toStdString().c_str());

        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"Chat","Message cound not be empty!");
    }
}


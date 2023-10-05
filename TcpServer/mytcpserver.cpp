#include "mytcpserver.h"
#include<QList>
#include"mytcpsocket.h"

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::GetInstance()
{
    static MyTcpServer instance;
    return instance;
}


void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"new client connected";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcp_socket_list.append(pTcpSocket);   //加入到链表中

    connect(pTcpSocket,SIGNAL(Offline(MyTcpSocket*)),this,SLOT(DeleteSocket(MyTcpSocket*)));
}

//转发
void MyTcpServer::Forward(const char *his_name, PDU *pdu)
{
    if(his_name == NULL || pdu == NULL) return ;
    QString str_name = his_name;
    for(int i =0;i<m_tcp_socket_list.size();i++)
    {
        if(str_name == m_tcp_socket_list.at(i)->GetName())
        {
            m_tcp_socket_list.at(i)->write((char*)pdu,pdu->all_PDU_size);
            break;
        }
    }
}

void MyTcpServer::DeleteSocket(MyTcpSocket *my_socket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcp_socket_list.begin();
    for(;iter != m_tcp_socket_list.end();iter++)
    {
        if(my_socket == *iter)
        {
            delete *iter;
            *iter = NULL;
            m_tcp_socket_list.erase(iter);
            break;
        }
    }
}

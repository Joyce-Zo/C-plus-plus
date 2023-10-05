#pragma once
#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QDebug>
#include"mytcpsocket.h"

//#include"protocol.h"
#include<QList>

class MyTcpSocket;
class MyTcpServer : public QTcpServer
{
    Q_OBJECT    //添加宏使其可以使用信号槽
public:
    MyTcpServer();

    static MyTcpServer& GetInstance();

    void incomingConnection(qintptr socketDescriptor);
    void Forward(const char* his_name,PDU* pdu);    //转发
public slots:
    void DeleteSocket(MyTcpSocket* socket);

private:
    QList<MyTcpSocket*> m_tcp_socket_list;//保存读取到的tcpsocket
};

#endif // MYTCPSERVER_H

#pragma once
#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include"protocol.h"
#include"operatedb.h"
#include"mytcpserver.h"
#include<QDir>
#include<QFile>
#include<QTimer>


class MyTcpServer;
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT    //添加使其支持信号槽
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString GetName();
    void CopyDir(QString src_dir,QString dest_dir);     //拷贝文件夹

signals:
    void Offline(MyTcpSocket* mysocket);

public slots:
    void ReceiveMsg();
    void ClientOffline();       //客户端离线槽函数
    void SendFileToClient();    //发送文件给客户端
private:
    QString m_str_name; //修改状态时，通过此来查找
    QFile m_file;       //要上传的文件
    qint64 m_file_total_size;   //文件总大小
    qint64 m_has_upload_size;   //已经上传的大小，与总大小对比
    bool m_is_update;   //是否已经更新

    QTimer* m_timer;    //定时器

};

#endif // MYTCPSOCKET_H

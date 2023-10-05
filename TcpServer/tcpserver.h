#ifndef TCPSERVER_H
#define TCPSERVER_H
#pragma once

#include <QWidget>
#include<QFile>
#include<QByteArray>
#include<QDebug>
#include<QMessageBox>
#include<QTcpSocket>
#include<QHostAddress>

QT_BEGIN_NAMESPACE
namespace Ui { class TcpServer; }
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();
    void LoadConfig();//读取配置文件

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::TcpServer *ui;
    QString m_strIp;    //ip地址
    quint16 m_usPort;   //无符号短整型    端口
};
#endif // TCPSERVER_H

#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#pragma once
#include <QWidget>
#include<QFile>
#include<QByteArray>
#include<QDebug>
#include<QMessageBox>
#include<QTcpSocket>
#include<QHostAddress>
#include"protocol.h"
#include"operatewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void LoadConfig();//读取配置文件
    static TcpClient & GetInstance();
    QTcpSocket& GetTcpSocket();
    QString LoginName();
    QString GetCurPath();   //返回当前路径
    void SetCurPath(QString m_pre_path);  //设置上级路径为当前路径

public slots:
    void ShowConnect();//显示连接状态
    void ReceiveMsg(); //接收TcpSocket数据

private slots:
    //void on_button_send_clicked();


    void on_pushButton_log_clicked();

    void on_pushButton_regist_clicked();

    void on_pushButton_log_out_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIp;    //ip地址
    quint16 m_usPort;   //无符号短整型    端口

    QTcpSocket m_tcp_socket;//用于连接服务器的对象，与服务器进行数据交互

    QString m_str_name; //登录的用户名

    QString m_cur_path; //保存的路径

    QFile m_file;       //保存文件
};
#endif // TCPCLIENT_H

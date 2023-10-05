#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"


TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    this->resize(400,300);
    LoadConfig();
    MyTcpServer::GetInstance().listen(QHostAddress(m_strIp),m_usPort);//开始监听
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::LoadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly))    //只读打开
    {
        QByteArray baData =file.readAll();  //读取所有数据，而非一行一行
        QString strData = baData.toStdString().c_str();
        file.close();
        strData.replace("\r\n"," ");        //替换回车换行符为空格
        QStringList strList=strData.split(" "); //以空格为切割符，切割后保存在链表里
        m_strIp=strList.at(0);  //获取链表0位置处数据即ip
        m_usPort=strList.at(1).toUShort();
    }
    else
    {
        QMessageBox::critical(this,"open config","Failed to open config");
    }
}


void TcpServer::on_pushButton_2_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}


void TcpServer::on_pushButton_3_clicked()
{
    this->close();
}


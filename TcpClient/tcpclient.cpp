#include "tcpclient.h"
#include "ui_tcpclient.h"
#include"chat.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    this->resize(500,350);
    LoadConfig();   //构造函数调用

    connect(&m_tcp_socket,&QTcpSocket::connected,this,ShowConnect);//信号连接

    connect(&m_tcp_socket,&QTcpSocket::readyRead,this,ReceiveMsg);//信号连接

    m_tcp_socket.connectToHost(QHostAddress(m_strIp),m_usPort);//连接服务器
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::LoadConfig()
{
    QFile file(":/client.config");
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

TcpClient &TcpClient::GetInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::GetTcpSocket()
{
    return m_tcp_socket;
}

QString TcpClient::LoginName()
{
    return m_str_name;
}

QString TcpClient::GetCurPath()
{
    return m_cur_path;
}

void TcpClient::SetCurPath(QString m_pre_path)
{
    m_cur_path = m_pre_path;
}

void TcpClient::ShowConnect()
{
    QMessageBox::information(this,"连接服务器","connection succeeded!");
}

void TcpClient::ReceiveMsg()
{
    //如果不是正在收数据的状态
    if(!OperateWidget::GetInstance().GetBook()->GetDownloadStatus())
    {
        ALL_SIZE all_PDU_size = 0;
        m_tcp_socket.read((char*)&all_PDU_size,sizeof(ALL_SIZE));
        ALL_SIZE real_msg_size = all_PDU_size-sizeof(PDU);
        PDU *pdu = CreatePDU(real_msg_size);
        m_tcp_socket.read((char*)pdu+sizeof(ALL_SIZE),all_PDU_size-sizeof(ALL_SIZE));

        switch(pdu->msg_type)
        {
            //***********************   登陆  /   注册   ******************************

        case ENUM_MSG_TYPE_REGIST_RESPOND:  //注册    回复
        {
            if(0 == strcmp(pdu->caData,REGIST_SUCCESSED))
            {
                QMessageBox::information(this,"Regist",REGIST_SUCCESSED);
            }
            else if(0 == strcmp(pdu->caData,REGIST_FAILED))
            {
                QMessageBox::warning(this,"Regist",REGIST_FAILED);
            }
            break;
        }

        case ENUM_MSG_TYPE_LOG_RESPOND:     //登录   回复
        {
            if(0 == strcmp(pdu->caData,LOGIN_SUCCESSED))
            {
                m_cur_path = QString("./%1").arg(m_str_name);
                QMessageBox::information(this,"Login",LOGIN_SUCCESSED);

                //设置标题名为登陆的用户名
                QString login_name = QString("TcpClient_user : %1").arg(m_str_name);
                OperateWidget::GetInstance().setWindowTitle(login_name);

                OperateWidget::GetInstance().show();    //调用静态对象引用以显示窗口
                this->hide();   //隐藏登录界面
            }
            else if(0 == strcmp(pdu->caData,LOGIN_FAILED))
            {
                QMessageBox::warning(this,"Login",LOGIN_FAILED);
            }
            break;
        }

            //***********************   好友      操作   ******************************

        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:  //查看所有在线用户    回复
        {
            OperateWidget::GetInstance().GetFriend()->ShowAllOnline(pdu);
            break;
        }

        case ENUM_MSG_TYPE_SEARCH_USER_RESPOND: //搜索用户    回复
        {
            if(0 == strcmp(SEARCH_NOT_FOUND,pdu->caData))
            {
                QMessageBox::information(this,"Search",
                                         QString("%1 :No such person").
                                         arg(OperateWidget::GetInstance().GetFriend()->m_str_search_name));
            }
            else if(0 == strcmp(SEARCH_NO,pdu->caData))
            {
                QMessageBox::information(this,"Search",QString("%1 :Offline").arg(OperateWidget::GetInstance().GetFriend()->m_str_search_name));
            }
            else if(0 == strcmp(SEARCH_YES,pdu->caData))
            {
                QMessageBox::information(this,"Search",QString("%1 :Online").arg(OperateWidget::GetInstance().GetFriend()->m_str_search_name));
            }
            break;
        }

        case ENUM_MSG_TYPE_ADD_REQUEST:         //添加好友  请求
        {
            char ca_name[32] = {'\0'};
            strncpy(ca_name,pdu->caData+32,32);
            int ret = QMessageBox::
                information(this,"Add Friend",QString("%1 want to add friend with you").
                                                arg(ca_name),QMessageBox::Yes,QMessageBox::No);
            PDU* res_pdu = CreatePDU(0);
            memcpy(res_pdu->caData,pdu->caData,64);

            if(QMessageBox::Yes == ret)
            {
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_ACCEPT;
            }
            else
            {
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_REFUSE;
            }
            m_tcp_socket.write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu=NULL;
            break;
        }

        case ENUM_MSG_TYPE_ADD_RESPOND:         //添加好友  回复
        {
            QMessageBox::information(this,"Add Friend",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_ADD_ACCEPT:          //添加好友  接受
        {
            char ca_his_name[32] = {'\0'};
            memcpy(ca_his_name, pdu->caData, 32);
            QMessageBox::information(this, "Add Friend", QString("Add friend with %1 successed!").arg(ca_his_name));
            break;
        }

        case ENUM_MSG_TYPE_ADD_REFUSE:          //添加好友  拒绝
        {
            char ca_his_name[32] = {'\0'};
            memcpy(ca_his_name, pdu->caData, 32);
            QMessageBox::information(this, "Add Friend", QString("Add friend with %1 failed:The other party rejected your request").arg(ca_his_name));
            break;
        }

        case ENUM_MSG_TYPE_REFRESH_RESPOND:     //刷新好友列表    回复
        {
            OperateWidget::GetInstance().GetFriend()->UpdateFriendList(pdu);
            break;
        }

        case ENUM_MSG_TYPE_DEL_REQUEST:         //删除好友  请求
        {
            char ca_name[32] = {'\0'};
            memcpy(ca_name,pdu->caData,32);
            QMessageBox::information(this,"Delete Friend",QString("%1 delete you as a frined").arg(ca_name));
            break;
        }

        case ENUM_MSG_TYPE_DEL_RESPOND:         //删除好友  回复
        {
            QMessageBox::information(this,"Delete Friend",DELETE_FREIEND_SUCCESSED);
            break;
        }

        case ENUM_MSG_TYPE_CHAT_REQUEST:        //私聊    请求
        {
            if(Chat::GetInstance().isHidden())
            {
                Chat::GetInstance().show();
            }
            char ca_my_name[32] = {'\0'};
            memcpy(ca_my_name,pdu->caData,32);
            QString str_my_name = ca_my_name;
            Chat::GetInstance().SetChatName(str_my_name);
            Chat::GetInstance().updateMsg(pdu);

            break;
        }

        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:  //群聊    请求
        {
            OperateWidget::GetInstance().GetFriend()->UpdateGroupMsg(pdu);
            break;
        }

            //***********************   文件  /   文件夹   ******************************

        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:  //创建文件夹
        {
            QMessageBox::information(this,"Create File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_REFRESH_DIR_RESPOND: //刷新文件夹（显示所有文件和目录）
        {
            OperateWidget::GetInstance().GetBook()->UpdateFileList(pdu);
            QString enter_dir =  OperateWidget::GetInstance().GetBook()->GetEnterDir();
            if(!enter_dir.isEmpty())
            {
                m_cur_path = m_cur_path+'/'+enter_dir;
            }
            break;
        }

        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:     //删除文件夹
        {
            QMessageBox::information(this,"Delete Dir",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: //重命名文件夹
        {
            QMessageBox::information(this,"Rename File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:   //双击进入文件夹
        {
            OperateWidget::GetInstance().GetBook()->ClearFailedEnterDirName();
            QMessageBox::information(this,"Enter Dir",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: //上传文件
        {
            QMessageBox::information(this,"Upload File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:    //删除文件
        {
            QMessageBox::information(this,"Delete File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND://下载文件
        {
            char file_name[32] = {'\0'};
            sscanf(pdu->caData,"%s:%lld",file_name,
                   &(OperateWidget::GetInstance().GetBook()->m_file_total_size));
            if(strlen(file_name) > 0 && OperateWidget::GetInstance().GetBook()->m_file_total_size > 0)
            {
                OperateWidget::GetInstance().GetBook()->SetDownloadStatus(true);
                m_file.setFileName(OperateWidget::GetInstance().GetBook()->GetPathToSaveFile());
                if(!m_file.open(QIODevice::WriteOnly))//没能打开文件
                {
                    QMessageBox::warning(this,"Download File","Failed to get path to save file");
                }
            }
            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:  //分享文件
        {
            QMessageBox::information(this,"Share File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_NOTICE:   //分享文件通知
        {
            char* path = new char[pdu->real_msg_size];
            memcpy(path,pdu->real_msg_data,pdu->real_msg_size);
            char *pos = strrchr(path,'/');

            if(pos != NULL)
            {
                pos++;
                QString note = QString
                               ("%1 share file:%2 \n Do you want to accept it?")
                                   .arg(pdu->caData).arg(pos);
                int ret = QMessageBox::question(this,"Share File",note);

                if(QMessageBox::Yes == ret)
                {
                    PDU* res_pdu = CreatePDU(pdu->real_msg_size);
                    res_pdu->msg_type = ENUM_MSG_TYPE_SHARE_FILE_NOTICE_RESPOND;
                    memcpy(res_pdu->real_msg_data,pdu->real_msg_data,pdu->real_msg_size);
                    QString my_name = TcpClient::GetInstance().LoginName();
                    strcpy(res_pdu->caData,my_name.toStdString().c_str());
                    m_tcp_socket.write((char*)res_pdu,res_pdu->all_PDU_size);
                }
            }
            break;
        }

        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:   //移动文件
        {
            QMessageBox::information(this,"Move File",pdu->caData);
            break;
        }

        case ENUM_MSG_TYPE_LOG_OUT_RESPOND:     //注销
        {
            if(0 == strcmp(pdu->caData,LOG_OUT_SUCCESSED))
            {
                QMessageBox::information(this,"Log_out",LOG_OUT_SUCCESSED);
            }
            else if(0 == strcmp(pdu->caData,LOG_OUT_FAILED))
            {
                QMessageBox::warning(this,"Log_out",LOG_OUT_FAILED);
            }
            break;
        }

        default:
            break;
        }
        free(pdu);
        pdu=NULL;
    }
    else    //是在收数据状态，则直接开始收数据
    {
        QByteArray buffer = m_tcp_socket.readAll();
        m_file.write(buffer);
        Books* book = OperateWidget::GetInstance().GetBook();
        book->m_has_download_size += buffer.size();
        m_tcp_socket.flush();
        if(book->m_file_total_size == book->m_has_download_size)
        {
            m_file.close();
            book->m_file_total_size = 0;
            book->m_has_download_size = 0;
            book->SetDownloadStatus(false);
            qDebug()<<"tcpclient下载完了";
            QMessageBox::information(this,"Download File",DOWNLOAD_FILE_SUCCESSED );
        }
        else if(book->m_file_total_size < book->m_has_download_size)
        {
            m_file.close();
            book->m_file_total_size = 0;
            book->m_has_download_size = 0;
            book->SetDownloadStatus(false);
            QMessageBox::critical(this,"Download File",DOWNLOAD_FILE_FAILED);
        }
    }
}


void TcpClient::on_pushButton_log_clicked()     //客户端发出登录请求
{
    QString str_name = ui->line_name->text();       //获取用户名
    QString str_passwd = ui->line_passwd->text();   //获取密码
    if(!str_name.isEmpty() && !str_passwd.isEmpty())
    {
        m_str_name = str_name;
        PDU* pdu = CreatePDU(0);    //产生pdu
        pdu->msg_type = ENUM_MSG_TYPE_LOG_REQUEST;   //发出注册请求
        strncpy(pdu->caData,str_name.toStdString().c_str(),32);//将用户名保存，前32字节全部留给用户名
        strncpy(pdu->caData+32,str_passwd.toStdString().c_str(),32);//将密码保存
        m_tcp_socket.write((char*)pdu,pdu->all_PDU_size);  //通过tcpsocket发送
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::critical(this,"Login","Failed Login: The name or password is empty!");
    }
}


void TcpClient::on_pushButton_regist_clicked()  //客户端发出注册请求
{
    QString str_name = ui->line_name->text();       //获取用户名
    QString str_passwd = ui->line_passwd->text();   //获取密码
    if(!str_name.isEmpty() && !str_passwd.isEmpty())
    {
        PDU* pdu = CreatePDU(0);    //产生pdu
        pdu->msg_type = ENUM_MSG_TYPE_REGIST_REQUEST;   //发出注册请求
        strncpy(pdu->caData,str_name.toStdString().c_str(),32);//将用户名保存，前32字节全部留给用户名
        strncpy(pdu->caData+32,str_passwd.toStdString().c_str(),32);//将用户名保存
        m_tcp_socket.write((char*)pdu,pdu->all_PDU_size);  //通过tcpsocket发送
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::critical(this,"Regist","Failed Regist: The name or password is empty!");
    }
}


void TcpClient::on_pushButton_log_out_clicked() //客户端发出注销请求
{
    QString str_name = ui->line_name->text();       //获取用户名
    QString str_passwd = ui->line_passwd->text();   //获取密码
    if(!str_name.isEmpty() && !str_passwd.isEmpty())
    {
        PDU* pdu = CreatePDU(0);    //产生pdu
        pdu->msg_type = ENUM_MSG_TYPE_LOG_OUT_REQUEST;   //发出注销请求
        strncpy(pdu->caData,str_name.toStdString().c_str(),32);//将用户名保存，前32字节全部留给用户名
        strncpy(pdu->caData+32,str_passwd.toStdString().c_str(),32);//将用户名保存
        m_tcp_socket.write((char*)pdu,pdu->all_PDU_size);  //通过tcpsocket发送
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::critical(this,"Log_out","Failed Log_out: The name or password is wrong!");
    }
}


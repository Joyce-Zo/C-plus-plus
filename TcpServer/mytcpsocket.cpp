#include "mytcpsocket.h"
#include"protocol.h"
#include<QDir>
#include<QIODevice>
#include<QFileInfoList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,&QTcpSocket::readyRead,\
            this,ReceiveMsg);//信号连接

    connect(this,&QTcpSocket::disconnected,\
            this,ClientOffline);//信号连接

    m_is_update = false;

    m_timer = new QTimer;
    connect(m_timer,QTimer::timeout,this,SendFileToClient);
}


QString MyTcpSocket::GetName()
{
    return m_str_name;
}

void MyTcpSocket::CopyDir(QString src_dir, QString dest_dir)
{
    QDir dir;
    dir.mkdir(dest_dir);
    dir.setPath(src_dir);
    QFileInfoList file_info_list = dir.entryInfoList();

    QString src_tmp,dest_tmp;
    for(int i=0;i<file_info_list.size();i++)
    {
        if(file_info_list[i].isFile())
        {
            src_tmp = src_dir+'/'+file_info_list[i].fileName();
            dest_tmp = dest_dir+'/'+file_info_list[i].fileName();
            QFile::copy(src_tmp,dest_tmp);
        }
        else if(file_info_list[i].isDir())
        {
            if(QString(".") == file_info_list[i].fileName() ||
                QString("..") == file_info_list[i].fileName()) continue;

            src_tmp = src_dir+'/'+file_info_list[i].fileName();
            dest_tmp = dest_dir+'/'+file_info_list[i].fileName();
            CopyDir(src_tmp,dest_tmp);//递归拷贝
        }
    }

}


void MyTcpSocket::ReceiveMsg()  //如果收到请求
{
    if(!m_is_update)

    {
        qDebug()<<"mytcpsocket当前获取到的数据数量"<<this->bytesAvailable();//当前获取到的数据数量
                                                                       ALL_SIZE all_PDU_size = 0;
        this->read((char*)&all_PDU_size,sizeof(ALL_SIZE));
        ALL_SIZE real_msg_size = all_PDU_size-sizeof(PDU);
        PDU *pdu = CreatePDU(real_msg_size);
        this->read((char*)pdu+sizeof(ALL_SIZE),all_PDU_size-sizeof(ALL_SIZE));

        switch(pdu->msg_type)
        {
            //***********************   登陆  /   注册   ******************************

        case ENUM_MSG_TYPE_REGIST_REQUEST:      //如果收到注册请求
        {
            char ca_name[32] = {'\0'};
            char ca_passwd[32] = {'\0'};
            strncpy(ca_name,pdu->caData,32);
            strncpy(ca_passwd,pdu->caData+32,32);
            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_REGIST_RESPOND;
            bool ret = OperateDB::GetInstance().HandleRegist(ca_name,ca_passwd);
            if(ret) //交给数据库函数处理
            {   //注册成功                                根据其返回值判断成功或失败
                strcpy(res_pdu->caData,REGIST_SUCCESSED);
                QDir dir;
            }
            else
            {   //注册失败
                strcpy(res_pdu->caData,REGIST_FAILED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送
            free(res_pdu);
            res_pdu=NULL;
            break;
        }

        case ENUM_MSG_TYPE_LOG_REQUEST:         //登录请求
        {
            char ca_name[32] = {'\0'};
            char ca_passwd[32] = {'\0'};
            strncpy(ca_name,pdu->caData,32);
            strncpy(ca_passwd,pdu->caData+32,32);
            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_LOG_RESPOND;
            // bool ret = ;
            if(OperateDB::GetInstance().HandleLog(ca_name,ca_passwd)) //交给数据库函数处理
            {   //登录成功                                根据其返回值判断成功或失败
                strcpy(res_pdu->caData,LOGIN_SUCCESSED);
                m_str_name = ca_name;//将名字保存，以便于后面修改状态
            }
            else
            {   //登录失败
                strcpy(res_pdu->caData,LOGIN_FAILED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送
            free(res_pdu);
            res_pdu=NULL;
            break;
        }

            //***********************   好友      操作   ******************************

        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:  //所有在线用户请求
        {
            QStringList ret = OperateDB::GetInstance().HandleAllOnline();

            ALL_SIZE real_msg_size = ret.size()*32;  //获取实际消息长度
            PDU* res_pdu = CreatePDU(real_msg_size);
            res_pdu->msg_type = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                memcpy((char*)(res_pdu->real_msg_data)+i*32,
                       ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送
            free(res_pdu);
            res_pdu=NULL;

            break;
        }

        case ENUM_MSG_TYPE_SEARCH_USER_REQUEST: //搜索用户请求
        {
            int ret = OperateDB::GetInstance().HandleSearchUser(pdu->caData);
            PDU * res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
            if(ret == -1)   strcpy(res_pdu->caData,SEARCH_NOT_FOUND);
            else if(ret == 0)   strcpy(res_pdu->caData,SEARCH_NO);
            else if(ret == 1)   strcpy(res_pdu->caData,SEARCH_YES);

            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
            free(res_pdu);
            res_pdu=NULL;

            break;
        }

        case ENUM_MSG_TYPE_ADD_REQUEST:         //添加好友
        {
            char ca_his_name[32] = {'\0'};  //获取对方名字
            char ca_name[32] = {'\0'};      //获取自己名字
            strncpy(ca_his_name,pdu->caData,32);
            strncpy(ca_name,pdu->caData+32,32);
            int ret = OperateDB::GetInstance().HandleAdd(ca_his_name,ca_name);
            PDU* res_pdu = NULL;
            if(ret == -1)
            {
                MyTcpServer::GetInstance().Forward(ca_his_name,pdu);
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_RESPOND;
                strcpy(res_pdu->caData,UNKNOW_ERROR);
                write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
                free(res_pdu);
                res_pdu=NULL;
            }//error

            else if (ret == 0)
            {
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_RESPOND;
                strcpy(res_pdu->caData,FRIEND_ALREADY);

                write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
                free(res_pdu);
                res_pdu=NULL;
            }//已是好友

            else if (ret == 1)
            {
                MyTcpServer::GetInstance().Forward(ca_his_name,pdu);
            }//非好友在线

            else if (ret == 2)
            {
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_RESPOND;
                strcpy(res_pdu->caData,FRIEND_NO_ONLINE);

                write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
                free(res_pdu);
                res_pdu=NULL;
            }//不在线

            else if (ret == 3)
            {
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_ADD_RESPOND;
                strcpy(res_pdu->caData,FRIEND_NO_EXISTED);

                write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
                free(res_pdu);
                res_pdu=NULL;
            }//不存在
            break;
        }

        case ENUM_MSG_TYPE_ADD_ACCEPT:          //接受 添加好友
        {
            char ca_his_name[32] = {'\0'};  //获取对方名字
            char ca_name[32] = {'\0'};      //获取自己名字
            strncpy(ca_his_name,pdu->caData,32);
            strncpy(ca_name,pdu->caData+32,32);
            OperateDB::GetInstance().HandleAddFriendAccept(ca_his_name,ca_name);
            MyTcpServer::GetInstance().Forward(ca_name,pdu);

            break;
        }

        case ENUM_MSG_TYPE_ADD_REFUSE:          //拒绝 添加好友
        {
            char ca_name[32] = {'\0'};
            strncpy(ca_name,pdu->caData+32,32);
            MyTcpServer::GetInstance().Forward(ca_name,pdu);
            break;
        }

        case ENUM_MSG_TYPE_REFRESH_REQUEST:     //刷新好友列表
        {
            char ca_name[32] = {'\0'};
            strncpy(ca_name,pdu->caData,32);
            QStringList ret = OperateDB::GetInstance().HandleRefreshFriendList(ca_name);
            ALL_SIZE  real_msg_size = ret.size()*32;
            PDU* res_pdu = CreatePDU(real_msg_size);
            res_pdu->msg_type = ENUM_MSG_TYPE_REFRESH_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                memcpy((char*)(res_pdu->real_msg_data)+i*32,
                       ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
            free(res_pdu);
            res_pdu=NULL;

            break;
        }

        case ENUM_MSG_TYPE_DEL_REQUEST:         //删除好友
        {

            char ca_my_name[32] = {'\0'};
            char ca_his_name[32] = {'\0'};
            strncpy(ca_my_name,pdu->caData,32);
            strncpy(ca_his_name,pdu->caData+32,32);
            OperateDB::GetInstance().HandleDelFriend(ca_my_name,ca_his_name);

            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_DEL_RESPOND;
            strcpy(res_pdu->caData,DELETE_FREIEND_SUCCESSED);
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送给客户端
            free(res_pdu);
            res_pdu=NULL;

            MyTcpServer::GetInstance().Forward(ca_his_name,pdu);
            break;
        }

        case ENUM_MSG_TYPE_CHAT_REQUEST:        //私聊
        {
            char ca_his_name[32] = {'\0'};
            memcpy(ca_his_name,pdu->caData+32,32);
            MyTcpServer::GetInstance().Forward(ca_his_name,pdu);
            break;
        }

        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:  //群聊
        {
            char ca_name[32] = {'\0'};
            strncpy(ca_name,pdu->caData,32);
            QStringList online_friend = OperateDB::GetInstance().HandleRefreshFriendList(ca_name);
            QString tmp;
            for(int i =0;i<online_friend.size();i++)
            {
                tmp = online_friend.at(i);
                MyTcpServer::GetInstance().Forward(tmp.toStdString().c_str(),pdu);
            }
            break;
        }

            //***********************   文件  /   文件夹   ******************************

        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:  //创建文件夹
        {
            QDir dir;
            QString cur_path = QString("%1").arg((char*)(pdu->real_msg_data));
            bool ret = dir.exists(cur_path);
            PDU* res_pdu = NULL;
            if(ret) //当前目录存在
            {
                char new_dir[32] = {'\0'};
                memcpy(new_dir,pdu->caData+32,32);
                QString new_path = cur_path+"/"+new_dir;
                ret = dir.exists(new_path);
                if(ret) //创建的文件名已存在
                {
                    res_pdu = CreatePDU(0);
                    res_pdu->msg_type = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(res_pdu->caData,FILE_EXISTED);
                }
                else    //文件名不存在
                {
                    dir.mkdir(new_path);
                    res_pdu = CreatePDU(0);
                    res_pdu->msg_type = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(res_pdu->caData,DIR_CREATE_SECCUEEED);
                }
            }
            else    //目录不存在
            {
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(res_pdu->caData,DIR_NOT_EXISTED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_REFRESH_DIR_REQUEST: //刷新文件夹（查看所有文件和目录）
        {
            char* cur_path = new char[pdu->real_msg_size];
            memcpy(cur_path,pdu->real_msg_data,pdu->real_msg_size);
            QDir dir(cur_path);
            QFileInfoList file_info_list = dir.entryInfoList();
            int file_count = file_info_list.size();
            PDU* res_pdu = CreatePDU(sizeof(FileInfo)*(file_count));
            res_pdu->msg_type = ENUM_MSG_TYPE_REFRESH_DIR_RESPOND;
            FileInfo*  file_info = NULL;
            QString str_file_name;
            for(int i=0;i<file_info_list.size();i++)
            {
                file_info = (FileInfo*)(res_pdu->real_msg_data)+i;
                str_file_name = file_info_list[i].fileName();
                memcpy(file_info->file_name,str_file_name.toStdString().c_str(),str_file_name.size());
                if(file_info_list[i].isDir())
                {
                    file_info->file_type = 0;   //文件夹
                }
                else if(file_info_list[i].isFile())

                {
                    file_info->file_type = 1;   //文件
                }
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:     //删除文件夹（非文件）
        {
            char file_name[32] = {'\0'};
            strcpy(file_name,pdu->caData);
            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);
            QString new_path = QString("%1/%2").arg(file_path).arg(file_name);

            QFileInfo file_info(new_path);
            bool ret = 0;
            if(file_info.isDir())
            {
                QDir dir;
                dir.setPath(new_path);
                ret = dir.removeRecursively();  //连着文件夹里的文件一起删除
            }
            else if(file_info.isFile()) //非文件夹，是文件
            {
                ret = 0;
            }

            //回复
            PDU * res_pdu = NULL;
            if(ret)
            {
                res_pdu = CreatePDU(strlen(DEL_DIR_SUCCESSED+1));
                res_pdu->msg_type =ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(res_pdu->caData,DEL_DIR_SUCCESSED,strlen(DEL_DIR_SUCCESSED));
            }
            else
            {
                res_pdu = CreatePDU(strlen(DEL_DIR_FAILED+1));
                res_pdu->msg_type =ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(res_pdu->caData,DEL_DIR_FAILED,strlen(DEL_DIR_FAILED));
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: //重命名文件夹
        {
            char old_name[32] = {'\0'};
            strcpy(old_name,pdu->caData);
            char* new_name = new char[pdu->real_msg_size];
            strcpy(new_name,pdu->caData+32);

            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);

            QString old_path = QString("%1/%2").arg(file_path).arg(old_name);
            QString new_path = QString("%1/%2").arg(file_path).arg(new_name);

            QDir dir;
            bool ret = dir.rename(old_path,new_path);
            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                strcpy(res_pdu->caData,RENAME_FILE_SUCCESSED);
            }
            else
            {
                strcpy(res_pdu->caData,RENAME_FILE_FAILED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:   //双击进入文件夹
        {
            char enter_name[32] = {'\0'};
            strcpy(enter_name,pdu->caData);

            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);

            QString path = QString("%1/%2").arg(file_path).arg(enter_name);
            QFileInfo file_info(path);
            PDU* res_pdu = NULL;
            if(file_info.isDir())
            {
                QDir dir(path);
                QFileInfoList file_info_list = dir.entryInfoList();
                int file_count = file_info_list.size();
                PDU* res_pdu = CreatePDU(sizeof(FileInfo)*(file_count));
                res_pdu->msg_type = ENUM_MSG_TYPE_REFRESH_DIR_RESPOND;
                FileInfo*  file_info = NULL;
                QString str_file_name;
                for(int i=0;i<file_info_list.size();i++)
                {
                    file_info = (FileInfo*)(res_pdu->real_msg_data)+i;
                    str_file_name = file_info_list[i].fileName();
                    memcpy(file_info->file_name,str_file_name.toStdString().c_str(),str_file_name.size());
                    if(file_info_list[i].isDir())
                    {
                        file_info->file_type = 0;   //文件夹
                    }
                    else if(file_info_list[i].isFile())

                    {
                        file_info->file_type = 1;   //文件
                    }
                }
                write((char*)res_pdu,res_pdu->all_PDU_size);
                free(res_pdu);
                res_pdu = NULL;
            }
            else if(file_info.isFile())
            {
                res_pdu = CreatePDU(0);
                res_pdu->msg_type = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(res_pdu->caData,ENTER_DIR_FAILED);
                write((char*)res_pdu,res_pdu->all_PDU_size);
                free(res_pdu);
                res_pdu = NULL;
            }
            break;
        }

        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: //上传文件
        {
            char file_name[32] = {'\0'};
            qint64 file_size = 0;
            sscanf(pdu->caData,"%s %lld",file_name,&file_size);

            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);
            QString path = QString("%1/%2").arg(file_path).arg(file_name);

            delete []file_path;
            file_path = NULL;

            m_file.setFileName(path);

            //以只读方式打开文件，不存在则自动创建
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_is_update = true;
                m_file_total_size = file_size;
                m_has_upload_size = 0;
            }
            break;
        }

        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:    //删除文件
        {
            char file_name[32] = {'\0'};
            strcpy(file_name,pdu->caData);
            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);
            QString new_path = QString("%1/%2").arg(file_path).arg(file_name);

            QFileInfo file_info(new_path);
            bool ret = 0;
            if(file_info.isDir())
            {
                ret = 0;

            }
            else if(file_info.isFile()) //非文件夹，是文件，才删除
            {
                QDir dir;
                ret = dir.remove(new_path);
            }
            //回复
            PDU * res_pdu = NULL;
            if(ret)
            {
                res_pdu = CreatePDU(strlen(DEL_FILE_SUCCESSED+1));
                res_pdu->msg_type =ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(res_pdu->caData,DEL_FILE_SUCCESSED,strlen(DEL_FILE_SUCCESSED));
            }
            else
            {
                res_pdu = CreatePDU(strlen(DEL_FILE_FAILED+1));
                res_pdu->msg_type =ENUM_MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(res_pdu->caData,DEL_FILE_FAILED,strlen(DEL_FILE_FAILED));
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST://下载文件
        {
            //获取文件路径
            char file_name[32] = {'\0'};
            strcpy(file_name,pdu->caData);
            char* file_path = new char[pdu->real_msg_size];
            memcpy(file_path,pdu->real_msg_data,pdu->real_msg_size);
            QString path = QString("%1/%2").arg(file_path).arg(file_name);

            delete []file_path;
            file_path = NULL;

            //读取用户及要下载的文件大小
            QFileInfo file_info(path);
            qint64 file_size = file_info.size();
            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(res_pdu->caData,"%s: %lld",file_name,file_size);

            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;

            m_file.setFileName(path);
            m_file.open(QIODevice::ReadOnly);
            m_timer->start(1000);   //开始定时器

            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:  //分享文件
        {
            char my_name[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caData,"%s%d",my_name,&num);
            int size = num*32;
            PDU* res_pdu = CreatePDU(pdu->real_msg_size-size);
            res_pdu->msg_type = ENUM_MSG_TYPE_SHARE_FILE_NOTICE;
            strcpy(res_pdu->caData,my_name);    //分享者
            memcpy(res_pdu->real_msg_data,
                   (char*)(pdu->real_msg_data)+size,pdu->real_msg_size-size);//分享的文件

            //接收者
            char receive_name[32] = {'\0'};
            for(int i=0;i<num;i++)
            {
                memcpy(receive_name,(char*)(pdu->real_msg_data)+i*32,32);
                MyTcpServer::GetInstance().Forward(receive_name,res_pdu);
            }
            free(res_pdu);
            res_pdu = NULL;

            //回复发送者
            res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(res_pdu->caData,SHARE_FILE_SUCCESSED);
            write((char*)res_pdu,res_pdu->all_PDU_size);

            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_NOTICE_RESPOND://分享文件 提示
        {
            QString receive_path = QString("./%1").arg(pdu->caData);
            QString share_file_path = QString("%1").arg((char*)(pdu->real_msg_data));
            int index = share_file_path.lastIndexOf('/');
            QString file_name = share_file_path.right(share_file_path.size()-index-1);

            receive_path = receive_path+'/'+file_name;


            QFileInfo file_info(share_file_path);
            if(file_info.isFile())
            {
                QFile::copy(share_file_path,receive_path);
            }
            else if(file_info.isDir())
            {
                CopyDir(share_file_path,receive_path);
            }
            break;
        }

        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:   //移动文件
        {
            char file_name[32] = {'\0'};
            int src_len = 0;
            int dest_len = 0;
            sscanf(pdu->caData,"%d%d%s",&src_len,&dest_len,file_name);

            char* src_path = new char[src_len+1];
            char* dest_path = new char[dest_len+32+1];

            memset(src_path,'\0',src_len+1);        //清空数据
            memset(dest_path,'\0',dest_len+1+32);

            memcpy(src_path,pdu->real_msg_data,src_len);
            memcpy(dest_path,(char*)(pdu->real_msg_data)+(src_len+1),dest_len);

            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo file_info(dest_path);
            if(file_info.isDir())
            {
                strcat(dest_path,"/");
                strcat(dest_path,file_name);

                bool ret = QFile::rename(src_path,dest_path);
                if(ret)
                {
                    strcpy(pdu->caData,MOVE_FILE_SUCCESSED);
                }
                else
                {
                    strcpy(pdu->caData,COMMON_ERR);
                }
            }
            else if(file_info.isFile())
            {
                strcpy(res_pdu->caData,MOVE_FILE_FAILED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_LOG_OUT_REQUEST:     //注销请求
        {
            char ca_name[32] = {'\0'};
            char ca_passwd[32] = {'\0'};
            strncpy(ca_name,pdu->caData,32);
            strncpy(ca_passwd,pdu->caData+32,32);
            PDU* res_pdu = CreatePDU(0);
            res_pdu->msg_type = ENUM_MSG_TYPE_LOG_OUT_RESPOND;
            if(OperateDB::GetInstance().HandleLogOut(ca_name,ca_passwd)) //交给数据库函数处理
            {   //注销成功                                根据其返回值判断成功或失败
                strcpy(res_pdu->caData,LOG_OUT_SUCCESSED);
            }
            else
            {   //注销失败
                strcpy(res_pdu->caData,LOG_OUT_FAILED);
            }
            write((char*)res_pdu,res_pdu->all_PDU_size);  //通过tcpsocket发送
            free(res_pdu);
            res_pdu=NULL;
            break;
        }

        default:
            break;
        }
        free(pdu);
        pdu=NULL;
    }

    else//上传数据
    {
        PDU * res_pdu = NULL;
        QByteArray buffer = readAll();
        m_file.write(buffer);
        m_has_upload_size += buffer.size();
        res_pdu = CreatePDU(0);
        res_pdu->msg_type = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if(m_file_total_size == m_has_upload_size)
        {
            m_file.close();
            m_is_update = false;

            strcpy(res_pdu->caData,UPLOAD_FILE_SUCCESSED);

            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
        }
        else if(m_file_total_size < m_has_upload_size)
        {
            m_file.close();
            m_is_update = false;

            strcpy(res_pdu->caData,UPLOAD_FILE_FAILED);

            write((char*)res_pdu,res_pdu->all_PDU_size);
            free(res_pdu);
            res_pdu = NULL;
        }
    }
}

void MyTcpSocket::ClientOffline()
{
    OperateDB::GetInstance().HandleOffline(m_str_name.toStdString().c_str());
    emit Offline(this);
}

//读取文件内容发送给客户端
void MyTcpSocket::SendFileToClient()
{
    //    m_timer->stop();
    char* data = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(data,4096);
        if(ret > 0 && ret <= 4096)
        {
            write(data,ret);
        }
        else if(ret == 0)   //数据读取完
        {
            qDebug()<<"mytcpsocket下载完了";
            m_file.close();
            m_timer->stop();
            break;
        }
        else if(ret < 0)
        {
            qDebug()<<"发送文件内容至客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []data;
    data = NULL;
}

 #include "friend.h"
#include"protocol.h"
#include"tcpclient.h"
#include"QInputDialog"
#include"chat.h"
#include<QMessageBox>



Friend::Friend(QWidget *parent)
    : QWidget{parent}
{

    m_show_msg =            new QTextEdit;      //显示信息
    m_show_friend_list =    new QListWidget;    //显示好友列表
    m_msg_input =           new QLineEdit;      //信息输入框

    m_del_friend =          new QPushButton("delete friend");   //删除好友
    m_refresh_friend_list = new QPushButton("refresh friend list"); //刷新好友列表
    m_show_online_user =    new QPushButton("show online user");    //显示在线用户
    m_search_user =         new QPushButton("search user");     //搜索用户
    m_send_msg =            new QPushButton("sending message");    //发送信息
    m_chat =                new QPushButton("chat");            //聊天

    QVBoxLayout* right_pbvbl = new QVBoxLayout;  //创建垂直布局
    //将下面几个按钮设在垂直布局中
    right_pbvbl->addWidget(m_del_friend);
    right_pbvbl->addWidget(m_refresh_friend_list);
    right_pbvbl->addWidget(m_show_online_user);
    right_pbvbl->addWidget(m_search_user);
    right_pbvbl->addWidget(m_chat);

    QHBoxLayout* top_hbl = new QHBoxLayout;  //创建水平布局
    //将下面几个按钮设在水平布局中
    top_hbl->addWidget(m_show_msg);
    top_hbl->addWidget(m_show_friend_list);
    top_hbl->addLayout(right_pbvbl);    //将垂直布局加入设为水平布局

    QHBoxLayout* msg_hbl = new QHBoxLayout;  //创建信息发送控件
    msg_hbl->addWidget(m_msg_input);
    msg_hbl->addWidget(m_send_msg);

    m_online = new Online;  //创建onlie对象

    QVBoxLayout* main = new QVBoxLayout;  //创建整个的布局
    main->addLayout(top_hbl);
    main->addLayout(msg_hbl);
    main->addWidget(m_online);  //添加按钮，但不显示
    m_online->hide();   //隐藏

    setLayout(main);

    connect(m_show_online_user,
            QPushButton::clicked,this,ShowOnline);   //连接显示在线用户键与显示在线用户信号
    connect(m_search_user,
            QPushButton::clicked,this,SearchUser);  //连接搜索键与搜索用户信号
    connect(m_refresh_friend_list,
            QPushButton::clicked,this,RefreshFriendList);//连接刷新键与刷新用户信号
    connect(m_del_friend,
            QPushButton::clicked,this,DeleteFriend);    //连接删除键和删除用户信号
    connect(m_chat,
            QPushButton::clicked,this,PrivateChat);    //连接私聊键和开启私聊信号
    connect(m_send_msg,
            QPushButton::clicked,this,GroupChat);    //连接聊天键和开启群聊信号

}
//显示在线用户
void Friend::ShowAllOnline(PDU *pdu)
{
    if(pdu ==NULL ) return ;
    m_online->ShowUser(pdu);
}

void Friend::UpdateFriendList(PDU *pdu)
{
    if(pdu ==NULL ) return ;
    ALL_SIZE ui_size = pdu->real_msg_size/32;
    char ca_name[32] = {'\0'};
    for(ALL_SIZE i = 0; i<ui_size;i++)
    {
        memcpy(ca_name,(char*)(pdu->real_msg_data)+i*32,32);
        m_show_friend_list->addItem(ca_name);
    }
}

void Friend::ShowOnline()
{
    if(m_online->isHidden())
    {
        m_online->show();
        PDU* pdu = CreatePDU(0);
        pdu->msg_type = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::GetInstance().GetTcpSocket().
            write((char*)pdu,pdu->all_PDU_size);   //发送请求信息
        free(pdu);
        pdu = NULL;
    }
    else    m_online->hide();
}

//更新群聊信息
void Friend::UpdateGroupMsg(PDU *pdu)
{
    QString str_msg = QString("%1 says: %2").arg(pdu->caData).arg((char*)(pdu->real_msg_data));
    m_show_msg->append(str_msg);
}

QListWidget *Friend::GetFriendList()
{
    return m_show_friend_list;
}

//搜索用户
void Friend::SearchUser()
{
    m_str_search_name = QInputDialog::getText(this,"Search","name");
    if(!m_str_search_name.isEmpty())
    {
        PDU* pdu = CreatePDU(0);
        memcpy(pdu->caData,m_str_search_name.toStdString().c_str(),
               m_str_search_name.size());
        pdu->msg_type = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        TcpClient::GetInstance().GetTcpSocket().
            write((char*)pdu,pdu->all_PDU_size);   //发送请求信息
        free(pdu);
        pdu = NULL;
    }
}

//刷新好友列表
void Friend::RefreshFriendList()
{
    m_show_friend_list->clear();

    QString str_name = TcpClient::GetInstance().LoginName();
    PDU* pdu = CreatePDU(0);
    pdu->msg_type = ENUM_MSG_TYPE_REFRESH_REQUEST;
    memcpy(pdu->caData,str_name.toStdString().c_str(),str_name.size());
    TcpClient::GetInstance().GetTcpSocket().
        write((char*)pdu,pdu->all_PDU_size);   //发送请求信息
    free(pdu);
    pdu = NULL;
}

//删除好友
void Friend::DeleteFriend()
{
    if(NULL != m_show_friend_list->currentItem())
    {
        QString str_his_name = m_show_friend_list->currentItem()->text();
        PDU* pdu = CreatePDU(0);
        pdu->msg_type = ENUM_MSG_TYPE_DEL_REQUEST;
        QString str_my_name = TcpClient::GetInstance().LoginName();
        memcpy(pdu->caData,str_my_name.toStdString().c_str(),str_my_name.size());
        memcpy(pdu->caData+32,str_his_name.toStdString().c_str(),str_his_name.size());
        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;
    }
}

//私聊
void Friend::PrivateChat()
{
    if(m_show_friend_list->currentItem() != NULL)
    {
        QString str_my_name = m_show_friend_list->currentItem()->text();
        Chat::GetInstance().SetChatName(str_my_name);
        if(Chat::GetInstance().isHidden())
        {
            Chat::GetInstance().show();
        }
    }
    else
    {
        QMessageBox::information(this,"Chat","Please choose someone to chat with");
    }
}

//群聊
void Friend::GroupChat()
{
    QString str_msg = m_msg_input->text();
    if(!str_msg.isEmpty())
    {
        PDU* pdu = CreatePDU(str_msg.size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString str_name = TcpClient::GetInstance().LoginName();
        strncpy(pdu->caData,str_name.toStdString().c_str(),str_name.size());
        strncpy((char*)(pdu->real_msg_data),str_msg.toStdString().c_str(),str_msg.size());
        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
    }
    else
    {
        QMessageBox::information(this,"Group Chat","Message cound not be empty!");
    }
        m_msg_input->clear();
}

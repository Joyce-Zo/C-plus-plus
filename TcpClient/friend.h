#ifndef FRIEND_H
#define FRIEND_H
#pragma once
#include <QWidget>

#include<QTextEdit>
#include<QListWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QDebug>
#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void ShowAllOnline(PDU* pdu);   //显示所有在线用户

    void UpdateFriendList(PDU* pdu);//更新好友列表

    void UpdateGroupMsg(PDU* pdu);  //更新群聊

    QString m_str_search_name;

    QListWidget* GetFriendList();   //获得好友列表

signals:

public slots:
    void ShowOnline();          //显示在线用户
    void SearchUser();          //搜索用户
    void RefreshFriendList();   //刷新好友列表
    void DeleteFriend();        //删除好友
    void PrivateChat();         //私聊
    void GroupChat();           //群聊
private:
    QTextEdit *m_show_msg;      //显示信息
    QListWidget* m_show_friend_list;    //显示好友列表
    QLineEdit* m_msg_input;     //信息输入框

    QPushButton* m_del_friend;  //删除好友
    QPushButton* m_refresh_friend_list; //刷新好友列表
    QPushButton* m_show_online_user;    //显示在线用户
    QPushButton* m_search_user; //搜索用户
    QPushButton* m_send_msg;    //发送信息
    QPushButton* m_chat;        //聊天按钮

    Online* m_online;


};

#endif // FRIEND_H

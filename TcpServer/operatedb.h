#pragma once

#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QMessageBox>
#include<QDebug>
#include<QStringList>


class OperateDB : public QObject
{
    Q_OBJECT
public:
    explicit OperateDB(QObject *parent = nullptr);

    static OperateDB& GetInstance();    //静态成员函数
    void InitDB();    //初始化数据库

    ~OperateDB();

    bool HandleRegist(const char* name,const char* passwd);     //处理注册请求
    bool HandleLog(const char* name,const char* passwd);        //处理登录请求
    bool HandleLogOut(const char* name,const char* passwd);        //处理注销请求

    void HandleOffline(const char* name);   //处理下线函数

    QStringList HandleAllOnline();  //显示所有在线用户

    int HandleSearchUser(const char* name); //搜索用户

    int HandleAdd(const char* his_name,const char* name);   //添加用户

    void HandleAddFriendAccept(const char* his_name,const char* name);  //接受添加用户

    QStringList HandleRefreshFriendList(const char* name);  //刷新用户列表

    bool HandleDelFriend(const char* my_name,const char* his_name);  //删除好友
signals:

private:
    QSqlDatabase m_db;  //连接数据库

};

#endif // OPERATEDB_H

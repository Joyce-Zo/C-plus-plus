#ifndef PROTOCOL_H
#define PROTOCOL_H
#pragma once
#endif // PROTOCOL_H

#include<stdlib.h>
#include<string>
#include<unistd.h>

//********************  user / friend  **************************

//  regist
#define REGIST_SUCCESSED "regist successed"
#define REGIST_FAILED "regist failed : The name has been existed!"

//  login
#define LOGIN_SUCCESSED "login successed"
#define LOGIN_FAILED "login failed : The name or passwd is wrong ,or has been online!"

//  search
#define SEARCH_YES "online"
#define SEARCH_NO "offline"
#define SEARCH_NOT_FOUND "no such person"

//  add
#define UNKNOW_ERROR "unknow error"
#define FRIEND_EXISTED "user existed"
#define FRIEND_NO_EXISTED "user no existed"
#define FRIEND_NO_ONLINE "user no online"
#define FRIEND_ONLIE "user online"
#define FRIEND_ALREADY "friend already"

//  del
#define DELETE_FREIEND_SUCCESSED "delete friend successed"


//********************  dir / file  **************************

//  create
#define DIR_NOT_EXISTED "current dir do not exist"
#define FILE_EXISTED "file alfready exist"
#define DIR_CREATE_SECCUEEED "create dir successfully"

//  del
#define DEL_DIR_SUCCESSED "Deleted folder successfully"
#define DEL_DIR_FAILED "Failed to delete folder"

//  rename
#define RENAME_FILE_SUCCESSED "File renamed successfully"
#define RENAME_FILE_FAILED "Failed to rename file"

//  upload_file
#define UPLOAD_FILE_SUCCESSED "File uploaded successfully"
#define UPLOAD_FILE_FAILED "File upload failed"

//  del_file
#define DEL_FILE_SUCCESSED "Deleted file successfully"
#define DEL_FILE_FAILED "Failed to delete file"

//  download_file
#define DOWNLOAD_FILE_SUCCESSED "File downloaded successfully"
#define DOWNLOAD_FILE_FAILED "File download failed"

//  share_file
#define SHARE_FILE_SUCCESSED "File shared successfully"
#define SHARE_FILE_FAILED "File share failed"

//  move_file
#define MOVE_FILE_SUCCESSED "File move successfully"
#define MOVE_FILE_FAILED "File move failed"

#define COMMON_ERR "unknow error"

//  log_out
#define LOG_OUT_SUCCESSED "Log out successfully"
#define LOG_OUT_FAILED "Log out failed: name or passwd is wrong!"

typedef unsigned int  ALL_SIZE; //总的数据大小
enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,

    ENUM_MSG_TYPE_REGIST_REQUEST,           //注册        请求
    ENUM_MSG_TYPE_REGIST_RESPOND,           //              回复

    ENUM_MSG_TYPE_LOG_REQUEST,              //登录        请求
    ENUM_MSG_TYPE_LOG_RESPOND,              //              回复

    //好友和用户操作
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,       //所有在线用户    请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,       //回复

    ENUM_MSG_TYPE_SEARCH_USER_REQUEST,      //搜索用户      请求
    ENUM_MSG_TYPE_SEARCH_USER_RESPOND,      //回复

    ENUM_MSG_TYPE_ADD_REQUEST,              //添加好友      请求
    ENUM_MSG_TYPE_ADD_RESPOND,              //回复

    ENUM_MSG_TYPE_ADD_ACCEPT,               //添加好友      接收
    ENUM_MSG_TYPE_ADD_REFUSE,               //              拒绝

    ENUM_MSG_TYPE_REFRESH_REQUEST,          //刷新好友列表    请求
    ENUM_MSG_TYPE_REFRESH_RESPOND,          //回复

    ENUM_MSG_TYPE_DEL_REQUEST,              //删除好友      请求
    ENUM_MSG_TYPE_DEL_RESPOND,              //回复

    ENUM_MSG_TYPE_CHAT_REQUEST,             //私聊        请求
    ENUM_MSG_TYPE_CHAT_RESPOND,             //回复

    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,       //群聊        请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,       //回复

    //文件和文件夹操作
    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,       //创建文件夹     请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,       //回复

    ENUM_MSG_TYPE_DEL_DIR_REQUEST,          //删除文件夹     请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,          //回复

    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,       //重命名文件夹    请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,       //回复

    ENUM_MSG_TYPE_REFRESH_DIR_REQUEST,       //刷新文件夹    请求
    ENUM_MSG_TYPE_REFRESH_DIR_RESPOND,       //回复

    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,        //进入文件夹    请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,        //回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,      //上传文件    请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,      //回复

    ENUM_MSG_TYPE_DEL_FILE_REQUEST,         //删除文件   请求
    ENUM_MSG_TYPE_DEL_FILE_RESPOND,         //回复

    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,    //下载文件   请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,    //回复

    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,       //分享文件    请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,       //回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTICE,        //通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTICE_RESPOND,//通知        回复

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,        //移动文件   请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,        //回复

    ENUM_MSG_TYPE_LOG_OUT_REQUEST,          //注销        请求
    ENUM_MSG_TYPE_LOG_OUT_RESPOND,          //            回复

    ENUM_MSG_TYPE_MAX=0X00ffffff
};

struct FileInfo
{
    char file_name[32]; //文件名
    int file_type;      //文件类型
};

struct PDU
{
    ALL_SIZE all_PDU_size;  //总的协议数据单元大小
    ALL_SIZE msg_type;      //消息类型
    char caData[64];        //文件名密码等
    ALL_SIZE real_msg_size; //实际消息大小
    int real_msg_data[];      //实际消息内容
};

PDU *CreatePDU(ALL_SIZE real_msg_size); //产生结构体

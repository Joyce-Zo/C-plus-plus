#pragma once
#ifndef BOOKS_H
#define BOOKS_H

#include <QWidget>
#include<QListWidget>
#include<QPushButton>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include"protocol.h"
#include<QTimer>


class Books : public QWidget
{
    Q_OBJECT
public:
    explicit Books(QWidget *parent = nullptr);
    void UpdateFileList(const PDU* pdu);    //更新文件列表
    void ClearFailedEnterDirName();         //清除未成功进入文件夹时仍保存的文件夹
    QString GetEnterDir();                  //成功则返回文件夹

    void SetDownloadStatus(bool status);    //设置m_is_download的状态
    bool GetDownloadStatus();               //获取m_is_download的状态

    QString GetPathToSaveFile();            //获取m_path_to_save_file
    QString GetFileToShare();               //获取m_file_to_share
    qint64 m_file_total_size;       //文件总大小
    qint64 m_has_download_size;     //已经下载的大小


signals:

public slots:
    void CreateDir();   //创建文件夹
    void RefreshFile(); //刷新文件：即查看文件
    void DelDir();      //删除文件夹
    void RenameFile();  //重命名文件
    void EnterDir(const QModelIndex &index);    //进入文件夹
    void BackToPrevious();  //返回上一级
    void SelectDestDir();   //选择目标文件夹
    void UploadFile();      //上传文件
    void DelFile();         //删除文件
    void DownloadFile();    //下载文件
    void ShareFile_book();  //共享文件
    void MoveFile();        //移动文件
    void UploadFileDelay();     //定时器

private:
    QListWidget * m_file_name_list; //文件列表
    QPushButton * m_back;           //返回
    QPushButton * m_create_dir;     //创建文件夹
    QPushButton * m_del_dir;        //删除文件夹
    QPushButton * m_rename_dir;     //重命名文件夹
    QPushButton * m_refresh_dir;    //刷新文件夹
    QPushButton * m_move_file_to_dir;   //文件移动的目标文件夹
    QPushButton * m_upload_file;    //上传文件
    QPushButton * m_del_file;       //删除文件
    QPushButton * m_download_file;  //下载文件
    QPushButton * m_share_file;     //分享文件
    QPushButton * m_move_file;      //移动文件

    QString m_enter_dir;            //进入的那个文件夹
    QString m_upload_file_path;     //打开的路径
    QString m_path_to_save_file;    //要保存文件的路径
    QString m_file_to_share;        //要分享的文件名
    QString m_file_to_move;         //要移动的文件名
    QString m_path_to_move_file;    //要移动文件的路径
    QString m_dest_dir;             //移动文件的目标路径

    QTimer * m_timer;               //定时器

    bool m_is_download;             //是否处于下载状态
};

#endif // BOOKS_H

#include "books.h"
#include"tcpclient.h"
#include<QMessageBox>
#include<QInputDialog>
#include<QFileDialog>
#include"operatewidget.h"
#include"sharefile.h"

Books::Books(QWidget *parent)
    : QWidget{parent}
{
    m_enter_dir.clear();
//       this->resize(500,500);
    m_is_download = false;

    m_timer = new QTimer;

    m_file_name_list    = new QListWidget;              //文件列表
    m_back              = new QPushButton("back");           //返回
    m_create_dir        = new QPushButton("create dir");     //创建文件夹
    m_del_dir           = new QPushButton("delete dir");     //删除文件夹
    m_rename_dir        = new QPushButton("rename dir");     //重命名文件夹
    m_refresh_dir       = new QPushButton("refresh dir");    //刷新文件夹
    m_move_file_to_dir  = new QPushButton("select dir");     //选择文件夹 去移动文件
    m_move_file_to_dir->setEnabled(false);

    QVBoxLayout * dir_vbl = new QVBoxLayout;    //垂直布局 : 文件夹操作
    dir_vbl->addWidget(m_back);
    dir_vbl->addWidget(m_create_dir);
    dir_vbl->addWidget(m_del_dir);
    dir_vbl->addWidget(m_rename_dir);
    dir_vbl->addWidget(m_refresh_dir);

    m_upload_file   = new QPushButton("upload file");    //上传文件
    m_del_file      = new QPushButton("delete file");    //删除文件
    m_download_file = new QPushButton("download file");  //下载文件
    m_share_file    = new QPushButton("share file");     //分享文件
    m_move_file     = new QPushButton("move file");      //移动文件

    QVBoxLayout * file_vbl = new QVBoxLayout;    //垂直布局 : 文件操作
    file_vbl->addWidget(m_upload_file);
    file_vbl->addWidget(m_del_file);
    file_vbl->addWidget(m_download_file);
    file_vbl->addWidget(m_share_file);
    file_vbl->addWidget(m_move_file);
    file_vbl->addWidget(m_move_file_to_dir);

    QHBoxLayout * main = new QHBoxLayout;
    main->addWidget(m_file_name_list);
    main->addLayout(dir_vbl);
    main->addLayout(file_vbl);


    setLayout(main);        //设为总的文件列表

    connect(m_create_dir,QPushButton::clicked,this,CreateDir);      //创建文件夹 信号连接
    connect(m_refresh_dir,QPushButton::clicked,this,RefreshFile);   //刷新文件夹 信号连接
    connect(m_del_dir,QPushButton::clicked,this,DelDir);            //删除文件夹 信号连接
    connect(m_rename_dir,QPushButton::clicked,this,RenameFile);     //重命名文件夹 信号连接
    connect(m_file_name_list,QAbstractItemView::doubleClicked,this,EnterDir);     //双击进入文件夹 信号连接
    connect(m_back,QPushButton::clicked,this,BackToPrevious);       //返回上一级 信号连接
    connect(m_move_file_to_dir,QPushButton::clicked,this,SelectDestDir);//移动文件 信号连接

    connect(m_upload_file,QPushButton::clicked,this,UploadFile);        //上传文件 信号连接
    connect(m_del_file,QPushButton::clicked,this,DelFile);              //删除文件 信号连接
    connect(m_download_file,QPushButton::clicked,this,DownloadFile);    //下载文件 信号连接
    connect(m_share_file,QPushButton::clicked,this,ShareFile_book);     //共享文件 信号连接
    connect(m_move_file,QPushButton::clicked,this,MoveFile);            //移动文件 信号连接


    connect(m_timer,QTimer::timeout,this,UploadFileDelay);          //定时器间隔上传文件，避免文件粘连

}

void Books::UpdateFileList(const PDU *pdu)
{
    if(!pdu)    return;

    QListWidgetItem* item_tmp = NULL;
    int row = m_file_name_list->count();
    while(m_file_name_list->count()>0)
    {
        item_tmp = m_file_name_list->item(row-1);
        m_file_name_list->removeItemWidget(item_tmp);
        delete item_tmp;
        row --;
    }
    FileInfo* file_info = NULL;
    int count = pdu->real_msg_size/sizeof(FileInfo);
    for(int i =0;i<count;i++)
    {
        file_info = (FileInfo*)(pdu->real_msg_data)+i;
        QListWidgetItem *item = new QListWidgetItem;
        if(!file_info->file_type)
            item->setIcon(QIcon(QPixmap(":/res/file.png")));
        else if(file_info->file_type)
            item->setIcon(QIcon(QPixmap(":/res/dir.png")));
        item->setText(file_info->file_name);
        m_file_name_list->addItem(item);
    }
}

void Books::ClearFailedEnterDirName()
{
    m_enter_dir.clear();
}

QString Books::GetEnterDir()
{
    return m_enter_dir;
}

void Books::CreateDir()
{
    QString new_dir_path = QInputDialog::getText(this,"Create DIR","new dir name:");
    if(!new_dir_path.isEmpty())
    {
        if(new_dir_path.size()<32)
        {
            QString my_name = TcpClient::GetInstance().LoginName();
            QString cur_path = TcpClient::GetInstance().GetCurPath();
            PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
            pdu->msg_type = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,my_name.toStdString().c_str(),my_name.size());
            strncpy(pdu->caData+32,new_dir_path.toStdString().c_str(),new_dir_path.size());
            memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
            TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this,"Create DIR","Dir name cound not exceed 32 characters!");
        }
    }
    else
    {
        QMessageBox::warning(this,"Create DIR","Dir name cound not be empty!");
    }
}

void Books::RefreshFile()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
    pdu->msg_type = ENUM_MSG_TYPE_REFRESH_DIR_REQUEST;
    strncpy((char*)(pdu->real_msg_data),cur_path.toStdString().c_str(),cur_path.size());
    TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
    free(pdu);
    pdu = NULL;
}

void Books::DelDir()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    QListWidgetItem* item = m_file_name_list->currentItem();
    if(!item)
    {
        QMessageBox::warning(this,"Delete Dir","Please select the folder to delete!");
    }
    else
    {
        QString del_name = item->text();
        PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,del_name.toStdString().c_str(),del_name.size());
        memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;
    }
}

void Books::RenameFile()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    QListWidgetItem* item = m_file_name_list->currentItem();
    if(!item)
    {
        QMessageBox::warning(this,"Rename File","Please select the file to rename");
    }
    else
    {
        QString old_name = item->text();
        QString new_name = QInputDialog::getText(this,"Rename File","Please select the file to rename");

        if(new_name.isEmpty())
        {
            QMessageBox::warning(this,"Rename File","The new file name cannot be empty!");
        }
        else
        {
            PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
            pdu->msg_type = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,old_name.toStdString().c_str(),old_name.size());
            strncpy(pdu->caData+32,new_name.toStdString().c_str(),new_name.size());
            memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
            TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
            free(pdu);
            pdu = NULL;
        }
    }
}

void Books::EnterDir(const QModelIndex &index)
{
    QString dir_name = index.data().toString();
    m_enter_dir = dir_name;
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
    pdu->msg_type = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,dir_name.toStdString().c_str(),dir_name.size());
    memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());

    TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
    free(pdu);
    pdu = NULL;
}

void Books::BackToPrevious()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    QString root_path = "./"+TcpClient::GetInstance().LoginName();
    if(cur_path == root_path)
    {
        QMessageBox::warning(this,"Back","Already in the root directory!");
    }
    else
    {
        int index = cur_path.lastIndexOf('/');  //以/为分隔符取出最后一个分隔符的索引值
        cur_path.remove(index,cur_path.size()-index);
        TcpClient::GetInstance().SetCurPath(cur_path);
        ClearFailedEnterDirName();
        RefreshFile();
    }
}

void Books::SelectDestDir()
{
    QListWidgetItem* cur_item = m_file_name_list->currentItem();
    if(cur_item != NULL)
    {
        QString dest_dir = cur_item->text();  //要移动的文件名
        QString cur_path = TcpClient::GetInstance().GetCurPath();
        m_dest_dir = cur_path+'/'+dest_dir;  //要移动文件所在的路径

        int src_len = m_path_to_move_file.size();
        int dest_len = m_dest_dir.size();
        PDU* pdu = CreatePDU(src_len+dest_len+2);
        pdu->msg_type = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",src_len,dest_len,m_file_to_move.toStdString().c_str());

        memcpy(pdu->real_msg_data,m_path_to_move_file.toStdString().c_str(),src_len);
        memcpy((char*)(pdu->real_msg_data)+(src_len+1),m_dest_dir.toStdString().c_str(),dest_len);

        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"Move File","Please select a file to move!");
    }
    m_move_file_to_dir->setEnabled(false);   //选择了目标文件夹才false
}

void Books::UploadFile()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    m_upload_file_path = QFileDialog::getOpenFileName();
    if(m_upload_file_path.isEmpty())
    {
        QMessageBox::warning(this,"Upload File","Please select a file!");
    }
    else
    {
        int index = m_upload_file_path.lastIndexOf('/');
        QString file_name = m_upload_file_path.right(m_upload_file_path.length()-index-1);

        QFile file(m_upload_file_path);
        qint64 file_size = file.size();

        QString cur_path = TcpClient::GetInstance().GetCurPath();
        PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
        sprintf(pdu->caData,"%s %lld",file_name.toStdString().c_str(),file_size);

        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;

        //启动定时器
        m_timer->start(1000);
    }
}

void Books::DelFile()
{
    QString cur_path = TcpClient::GetInstance().GetCurPath();
    QListWidgetItem* item = m_file_name_list->currentItem();
    if(!item)
    {
        QMessageBox::warning(this,"Delete File","Please select the file to delete!");
    }
    else
    {
        QString del_name = item->text();
        PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,del_name.toStdString().c_str(),del_name.size());
        memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);
        free(pdu);
        pdu = NULL;
    }
}

void Books::DownloadFile()
{
    QListWidgetItem* item = m_file_name_list->currentItem();
    if(!item)
    {
        QMessageBox::warning(this,"Download File","Please select the file to download!");
    }
    else
    {
        QString path_to_save_file =  QFileDialog::getSaveFileName();
        if(path_to_save_file.isEmpty())
        {
            QMessageBox::warning(this,"Download File","Please select a directory to save the file!");
            m_path_to_save_file.clear();
        }
        else
        {
            m_path_to_save_file = path_to_save_file;
        }
        QString cur_path = TcpClient::GetInstance().GetCurPath();
        PDU* pdu = CreatePDU(cur_path.toUtf8().size()+1);
        pdu->msg_type = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString file_name = item->text();
        strcpy(pdu->caData,file_name.toStdString().c_str());
        memcpy(pdu->real_msg_data,cur_path.toStdString().c_str(),cur_path.size());
        TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);

        free(pdu);
        pdu = NULL;
    }
}

void Books::ShareFile_book()
{
    QListWidgetItem* item = m_file_name_list->currentItem();
    if(!item)
    {
        QMessageBox::warning(this,"Share File","Please select the file to share!");
        return;
    }
    else
    {
        m_file_to_share = item->text();
    }
    Friend* pfriend = OperateWidget::GetInstance().GetFriend();
    QListWidget* friend_list = pfriend->GetFriendList();
    ShareFile::GetInstance().UpdateFriend(friend_list);
    if(ShareFile::GetInstance().isHidden())
    {
        ShareFile::GetInstance().show();
    }
}

void Books::MoveFile()
{
    QListWidgetItem* cur_item = m_file_name_list->currentItem();
    if(cur_item != NULL)
    {
        m_file_to_move = cur_item->text();      //要移动的文件名
        QString cur_path = TcpClient::GetInstance().GetCurPath();
        m_path_to_move_file = cur_path+'/'+m_file_to_move;  //要移动文件所在的路径

        m_move_file_to_dir->setEnabled(true);   //选择了目标文件夹才true
    }
    else
    {
        QMessageBox::warning(this,"Move File","Please select a file to move!");
    }
}

void Books::UploadFileDelay()
{
    m_timer->stop();
    QFile file(m_upload_file_path);
    if(file.open(QIODevice::ReadOnly))
    {
        char* buffer = new char[4096];
        qint64 ret = 0;
        while(true)
        {
            ret = file.read(buffer,4096);
            if(ret > 0 && ret <= 4096)   //数据在范围内
            {
                TcpClient::GetInstance().GetTcpSocket().write(buffer,ret);
            }
            else if(ret == 0)   //读到0，代表 读完了
            {
                break;
            }
            else    //读取失败
            {
                QMessageBox::warning(this,"Upload File","Error: Failed to read file!");
                break;
            }
        }
        file.close();
        delete []buffer;
        buffer = NULL;
    }
    else
    {
        QMessageBox::warning(this,"Upload File","File upload failed");
        return;
    }
}

void Books::SetDownloadStatus(bool status)
{
    m_is_download = status;
}

bool Books::GetDownloadStatus()
{
    return m_is_download;
}

QString Books::GetPathToSaveFile()
{
    return m_path_to_save_file;
}

QString Books::GetFileToShare()
{
    return m_file_to_share;
}

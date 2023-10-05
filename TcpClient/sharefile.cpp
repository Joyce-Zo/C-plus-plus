#include "sharefile.h"
#include"tcpclient.h"
#include"operatewidget.h"


ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_select_all = new QPushButton("select all");                 //全选
    m_cancel_select_all = new QPushButton("cancel select all"); //取消全选
    //将这两个按钮设为水平布局
    QHBoxLayout* top_hbl = new QHBoxLayout;
    top_hbl->addWidget(m_select_all);
    top_hbl->addWidget(m_cancel_select_all);
    top_hbl->addStretch();//将按钮置为最左边

    m_confirm = new QPushButton("confirm");           //确定操作
    m_cancel = new QPushButton("cancel");             //取消操作
    //将这两个按钮设为水平布局
    QHBoxLayout* down_hbl = new QHBoxLayout;
    down_hbl->addWidget(m_confirm);
    down_hbl->addWidget(m_cancel);

    m_show_widget = new QScrollArea;                //显示显示好友列表的控件
    m_show_user = new QWidget;                      //显示好友列表
    m_friend_vbl = new QVBoxLayout (m_show_user);

    m_manage_all_friend = new QButtonGroup(m_show_user);        //管理所有好友
    m_manage_all_friend->setExclusive(false);       //设置可以多选

    QVBoxLayout* main_vbl = new QVBoxLayout;        //将top_vbl/down_vbl和show_user合并为垂直布局
    main_vbl->addLayout(top_hbl);
    main_vbl->addWidget(m_show_widget);
    main_vbl->addLayout(down_hbl);
    setLayout(main_vbl);

    connect(m_cancel_select_all,QPushButton::clicked,this,CancelSelect);
    connect(m_cancel,QPushButton::clicked,this,Cancel);
    connect(m_confirm,QPushButton::clicked,this,Confirm);
    connect(m_select_all,QPushButton::clicked,this,SelectAll);
}

ShareFile &ShareFile::GetInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::UpdateFriend(QListWidget *friend_list)
{
    if(!friend_list)    return;
    //移除上次共享时显示的好友列表
    QAbstractButton* tmp =NULL;
    QList<QAbstractButton*> pre_friend_list = m_manage_all_friend->buttons();
    for(int i=0;i<pre_friend_list.size();i++)
    {
        tmp = pre_friend_list[i];
        m_friend_vbl->removeWidget(tmp);
        m_manage_all_friend->removeButton(tmp);
        pre_friend_list.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }

    QCheckBox* check = NULL;
    for(int i=0;i<friend_list->count();i++)
    {
        check = new QCheckBox(friend_list->item(i)->text());
        m_friend_vbl->addWidget(check);
        m_manage_all_friend->addButton(check);
    }
    m_show_widget->setWidget(m_show_user);
}

void ShareFile::test()
{
    QVBoxLayout* p = new QVBoxLayout(m_show_user);

    QCheckBox* check = NULL;
    for(int i=0;i<15;i++)
    {
        check = new QCheckBox("jack");
        p->addWidget(check);
        m_manage_all_friend->addButton(check);
    }
    m_show_widget->setWidget(m_show_user);

}

void ShareFile::SelectAll()
{
    QList<QAbstractButton*> pblist = m_manage_all_friend->buttons();
    for(int i=0;i<pblist.size();i++)
    {
        if(!pblist[i]->isChecked())
        {
            pblist[i]->setChecked(true);
        }
    }
}

void ShareFile::CancelSelect()
{
    QList<QAbstractButton*> pblist = m_manage_all_friend->buttons();
    for(int i=0;i<pblist.size();i++)
    {
        if(pblist[i]->isChecked())
        {
            pblist[i]->setChecked(false);
        }
    }
}

void ShareFile::Confirm()
{
    QString my_name = TcpClient::GetInstance().LoginName();     //分享者
    QString cur_path = TcpClient::GetInstance().GetCurPath();   //当前路径
    QString file_name_to_be_share = OperateWidget::GetInstance().GetBook()->GetFileToShare();

    QString path = cur_path+"/"+file_name_to_be_share;
    QList<QAbstractButton*> pblist = m_manage_all_friend->buttons();
    int person_count = 0;
    for(int i=0;i<pblist.size();i++)
    {
        if(pblist[i]->isChecked())
        {
            person_count++;
        }
    }

    PDU* pdu = CreatePDU(32*person_count+path.size()+1);
    pdu->msg_type = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",my_name.toStdString().c_str(),person_count);
    int j=0;
    for(int i=0;i<pblist.size();i++)
    {
        if(pblist[i]->isChecked())
        {
            memcpy((char*)(pdu->real_msg_data)+j*32,
                   pblist[i]->text().toStdString().c_str(),pblist[i]->text().size());
            j++;
        }
    }
    memcpy((char*)(pdu->real_msg_data)+person_count*32,path.toStdString().c_str(),path.size());

    TcpClient::GetInstance().GetTcpSocket().write((char*)pdu,pdu->all_PDU_size);

    free(pdu);
    pdu = NULL;
    hide();
}

void ShareFile::Cancel()
{
    hide();
}

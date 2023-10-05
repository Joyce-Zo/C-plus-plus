#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include<QPushButton>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QButtonGroup>
#include<QScrollArea>
#include<QCheckBox>
#include<QListWidget>


class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &GetInstance();

    void UpdateFriend(QListWidget* friend_list);
    void test();
signals:

public slots:
    void SelectAll();
    void CancelSelect();
    void Confirm();
    void Cancel();

private:
    QPushButton * m_select_all;         //全选
    QPushButton * m_cancel_select_all;  //取消全选

    QPushButton * m_confirm;            //确定操作
    QPushButton * m_cancel;             //取消操作

    QScrollArea * m_show_widget;         //显示显示好友列表的控件
    QWidget     * m_show_user;              //显示好友列表
    QVBoxLayout * m_friend_vbl;          //显示好友列表的垂直布局
    QButtonGroup* m_manage_all_friend;  //管理所有好友
};

#endif // SHAREFILE_H

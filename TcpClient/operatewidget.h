#ifndef OPERATEWIDGET_H
#define OPERATEWIDGET_H

#include <QWidget>
#include<QListWidget>
#include"friend.h"
#include "books.h"
#include<QStackedWidget>
#pragma once
class OperateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OperateWidget(QWidget *parent = nullptr);
    static OperateWidget & GetInstance();

    Friend * GetFriend();
    Books* GetBook();
signals:

public slots:

private:
    QListWidget* m_p_list_widget;
    Friend* m_friend;
    Books* m_book;

    QStackedWidget* m_SW;   //堆栈窗口，一次显示一个

};

#endif // OPERATEWIDGET_H

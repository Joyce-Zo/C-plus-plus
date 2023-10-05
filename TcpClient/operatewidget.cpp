#include "operatewidget.h"

OperateWidget::OperateWidget(QWidget *parent)
    : QWidget{parent}
{
    this->resize(700,450);
    m_p_list_widget = new QListWidget(this);
    m_p_list_widget->addItem("Frineds");
    m_p_list_widget->addItem("Books");

    m_friend = new Friend;
    m_book = new Books;

    m_SW = new QStackedWidget;  //创建堆栈创建
    m_SW->addWidget(m_friend);  //不指定顺序，默认显示第一个窗口
    m_SW->addWidget(m_book);

    QHBoxLayout* main = new QHBoxLayout;//水平布局
    main->addWidget(m_p_list_widget);
    main->addWidget(m_SW);

    setLayout(main);

    connect(m_p_list_widget,QListWidget::currentRowChanged,
            m_SW,QStackedWidget::setCurrentIndex);
}

OperateWidget &OperateWidget::GetInstance()
{
    static OperateWidget instance;
    return instance;
}

Friend *OperateWidget::GetFriend()
{
    return m_friend;
}

Books *OperateWidget::GetBook()
{
    return m_book;
}

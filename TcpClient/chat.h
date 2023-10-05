#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include"protocol.h"

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

    static Chat &GetInstance();
    void SetChatName(QString str_name);
    void updateMsg(const PDU *pdu);
private slots:
    void on_pushButton_send_msg_clicked();

private:
    Ui::Chat *ui;
    QString m_str_his_name;    //保存聊天的名字
    QString m_str_my_name;
};

#endif // CHAT_H

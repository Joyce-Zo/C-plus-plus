#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include"protocol.h"
#pragma once
namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();

    void ShowUser(PDU* pdu);

private slots:
    void on_pushButton_add_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H

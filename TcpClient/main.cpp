#include "tcpclient.h"

#include <QApplication>
//#include"books.h"
#include"sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font("Times",16,QFont::Bold);
    a.setFont(font);

    TcpClient::GetInstance().show();

//    Books w;
//    w.show();

//    ShareFile w;
//    w.test();
//    w.show();

    return a.exec();
}

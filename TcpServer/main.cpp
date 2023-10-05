#include "tcpserver.h"
#include"operatedb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer w;
    w.show();
    OperateDB::GetInstance().InitDB();

    return a.exec();
}

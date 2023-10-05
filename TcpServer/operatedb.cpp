#include "operatedb.h"

OperateDB::OperateDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OperateDB &OperateDB::GetInstance()
{
    static OperateDB instance;//定义静态对象
    return instance;    //每次调用返回
}

void OperateDB::InitDB()    //初始化数据库
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("C:\\Users\\HUIO\\Desktop\\Me\\QT\\progect\\cloud\\TcpServer\\cloud.db");
    if(m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%2").
                           arg(query.value(0).toString()).
                           arg(query.value(1).toString()).
                           arg(query.value(2).toString());
            qDebug()<<data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"Open DataBase","Failed Open DataBase!");
    }
}

OperateDB::~OperateDB()
{
    m_db.close();
}

//注册
bool OperateDB::HandleRegist(const char *name, const char *passwd)  //处理注册请求
{
    if(name == NULL || passwd == NULL)  return false;
    QString data = QString("insert into userInfo(name,passwd) "
                           "values(\'%1\',\'%2\')").arg(name).arg(passwd);//字符串拼接，并插入到数据库中
    QSqlQuery query;
    return query.exec(data);   //将数据传入
}

//登录
bool OperateDB::HandleLog(const char *name, const char *passwd)
{
    if(name == NULL || passwd == NULL)  return false;
    QString data = QString("select * from userInfo where "
                           "name=\'%1\' and passwd=\'%2\' "
                           "and online=0").arg(name).arg(passwd);//字符串拼接，并查询数据库中
    QSqlQuery query;
    query.exec(data);   //将数据传入
    if(query.next())//如果next()获取到数据，则返回真，否则返回假
    {                   //登陆后将online设为1
        data = QString("update userInfo set online=1 where "
                       "name=\'%1\' and passwd=\'%2\'").arg(name).arg(passwd);
        QSqlQuery query;
        query.exec(data);   //将数据传入
        return true;
    }
    else
    {
        return false;
    }
}

//注销
bool OperateDB::HandleLogOut(const char *name, const char *passwd)
{
    if(name == NULL || passwd == NULL)  return false;
    //查询用户信息
    QString data1 = QString("select * from userInfo where "
                           "name=\'%1\' and passwd=\'%2\' "
                           "and online=0").arg(name).arg(passwd);//字符串拼接，并查询数据库中
    QSqlQuery query1;
    query1.exec(data1);   //将数据传入

    //查询用户对应的id，以删除好友信息
    QString data2 = QString("select id from userInfo where "
                           "name=\'%1\' and passwd=\'%2\' "
                           "and online=0").arg(name).arg(passwd);//字符串拼接，并查询数据库中

    QSqlQuery query2;
    query2.exec(data2);   //将数据传入

    if(query1.next() && query2.next())//如果next1()和next2()获取到数据，则返回真，否则返回假
    {
        //删除用户信息
        data1 = QString("delete from userInfo where "
                       "name=\'%1\' and passwd=\'%2\'").arg(name).arg(passwd);
        QSqlQuery query1;
        query1.exec(data1);   //将数据传入

        //删除好友信息
        int id = query2.value(0).toInt();
        data2 = QString("delete from friendInfo where id = '\%1\'").arg(id);
        QSqlQuery query2;
        query2.exec(data2);   //将数据传入
        return true;
    }
    else
    {
        return false;
    }
}

//设置离线状态
void OperateDB::HandleOffline(const char *name) //设置在线状态为0
{
    if(name == NULL)  return;
    QString data = QString("update userInfo set online=0 where "
                           "name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);   //将数据传入
}

//所有在线用户
QStringList OperateDB::HandleAllOnline()
{
    QString data = QString("select name from userInfo where online=1");
    QSqlQuery query;
    query.exec(data);   //将数据传入

    QStringList result;
    result.clear();

    while(query.next()) //遍历查询
    {
        result.append(query.value(0).toString());   //有则存储
    }
    return result;
}

//搜索用户
int OperateDB::HandleSearchUser(const char *name)
{
    if(name == NULL) return -1;
    QString data = QString("select online "
                           "from userInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);   //将数据传入
    if(query.next())
    {
        return query.value(0).toInt();
    }
    else    return -1;
}

//添加用户
int OperateDB::HandleAdd(const char *his_name, const char *name)
{
    if(his_name == NULL || name == NULL)
    {
        return -1;  //error
    }
    QString data = QString("select * from friendId where "
                           "(id in (select id from userInfo where name=\'%1\') "
                           "and friendId = (select id from userInfo "
                           "where name=\'%2\'))"
                           " or (id in (select id from userInfo where name=\'%3\') "
                           "and friendId = (select id from userInfo "
                           "where name=\'%4\'))"
                           ).arg(his_name).arg(name).arg(name).arg(his_name);
    QSqlQuery query;
    query.exec(data);   //将数据传入
    if(query.next())    return 0;   //双方已是好友
    else
    {
        data = QString("select online from userInfo where name=\'%1\'").arg(his_name);
        QSqlQuery query;
        query.exec(data);
        if (query.next())
        {
            int ret = query.value(0).toInt();
            if (1 == ret)
            {
                return 1;   //在线
            }
            else if (0 == ret)
            {
                return 2;  //不在线
            }
        }
        else
        {
            return 3;   //不存在
        }
    }
}

//接受添加好友
void OperateDB::HandleAddFriendAccept(const char *his_name, const char *name)
{
    if (NULL == his_name || NULL == name)   return;
    QString data = QString("insert into friendInfo(id, friendId) "
                           "values((select id from userInfo where name=\'%1\'), "
                           "(select id from userInfo where name=\'%2\'))")
                       .arg(his_name).arg(name);
    QSqlQuery query;
    query.exec(data);
}

//刷新好友列表
QStringList OperateDB::HandleRefreshFriendList(const char *name)
{
    QStringList str_friend_list;
    str_friend_list.clear();
    if(name == NULL) return str_friend_list;
    QString data = QString("select name from userInfo where online=1 "
                           "and id in (select id from friendInfo "
                           "where friendId=(select id from userInfo "
                           "where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        str_friend_list.append(query.value(0).toString());
    }

    data = QString("select name from userInfo where online=1 "
                   "and id in (select friendId from friendInfo "
                   "where id=(select id from userInfo "
                   "where name=\'%1\'))").arg(name);
    query.exec(data);
    while(query.next())
    {
        str_friend_list.append(query.value(0).toString());
    }
    return str_friend_list;
}

//删除好友
bool OperateDB::HandleDelFriend(const char *my_name, const char *his_name)
{
    if(my_name == NULL || his_name == NULL)  return false;
    QString data = QString("delete from friendInfo where"
                           " id=(select id from userInfo  where"
                           " name=\'%1\') and friendId=(select id from userInfo "
                           "where name=\'%2\')").arg(my_name).arg(his_name);
    QSqlQuery query;
    query.exec(data);

    data = QString("delete from friendInfo where"
                   " id=(select id from userInfo  where"
                   " name=\'%1\') and friendId=(select id from userInfo "
                   "where name=\'%2\')").arg(his_name).arg(my_name);
    query.exec(data);

    return true;
}

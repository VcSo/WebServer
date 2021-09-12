#include "Httpconn.h"
int Http::m_user_count = 0;
int Http::m_epollfd = -1;
std::map<std::string, std::string> users;

Http::Http()
{

}

Http::~Http()
{

}

void Http::init_mysqlresult(ConnSql *m_sql)
{
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, m_sql);

    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }
}
#include "Httpconn.h"

#include <iostream>

int Http::m_user_count = 0;
int Http::m_epollfd = -1;
std::map<std::string, std::string> users;

Http::Http() : m_close_log(true)
{

}

Http::~Http()
{

}

void Http::init_mysqlresult(ConnSql *m_sql)
{
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, m_sql);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        users[temp1] = temp2;
    }
}

bool Http::read_once()
{

}
#include "Sqlconn.h"

SqlConnPool::SqlConnPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::init(const char *host, int port, std::string username, std::string password, std::string database, int sqlconn_num)
{
    static_assert(sqlconn_num > 0, "sqlconn_num <= 0");

    for(int i = 0; i < sqlconn_num; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if(!sql)
        {
            //LOG_ERROR("MySql init error!");
            static_assert(sql, "sql init error");
        }

        sql = mysql_real_connect(sql, host, username.c_str(), password.c_str(), database.c_str(), port, nullptr, 0);
        if(sql == nullptr)
        {
            //LOG_ERROR("MySql connect error!");
            std::cout << "connect error" << std::endl;
            continue;
        }
        m_connQue.push(sql);
    }
}
//
// Created by Vcvc on 2021/8/20.
//

#include "connSql.h"

connSql::connSql()
{
    m_free_conn = 0;
    m_cur_conn = 0;
}

connSql::~connSql()
{
    while(!sql_que.empty())
    {
        MYSQL *tmp = sql_que.front();
        sql_que.pop();
        mysql_close(tmp);
    }
    m_free_conn = 0;
    m_cur_conn = 0;
}

void connSql::init(std::string host, std::string username, std::string password, std::string database, int sqlnum)
{

    m_sqlnum = sqlnum;
    for(int i = 0; i < sqlnum; i++)
    {
        MYSQL *con = nullptr;
        con = mysql_init(con);

        if(con == nullptr)
        {
            //log error
            i--;
            continue;
        }

        con = mysql_real_connect(con, host.c_str(), username.c_str(), password.c_str(), database.c_str(), 3306, NULL, 0);
        if(con == nullptr)
        {
            //log error
            std::cerr << "mysql connect error " << std::endl;
            return;
        }

        sql_que.push(con);
        m_free_conn++;
    }
}

connSql *connSql::get_instance()
{
    static connSql sqlcon;
    return &sqlcon;
}
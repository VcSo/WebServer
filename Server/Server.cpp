#include "Server.h"

Server::Server(int port, std::string host, std::string username, std::string password, std::string database,
                bool uselog, int linger, int mode, int sqlnum, int threadnum, int actor, int logmode)
                : m_port(port), m_host(host), m_username(username), m_password(password), m_database(database),
                    m_uselog(uselog), m_linger(linger), et(mode), m_sql_num(sqlnum), m_thead_num(threadnum), m_actor(actor), m_log_mode(logmode)
{

}

Server::~Server()
{}

void Server::setsql()
{
    m_consql = connSql::get_instance();
    m_consql->init(m_host, m_username, m_password, m_database, m_sql_num);
}

void Server::setlog(std::string path)
{
    m_log_path = path;
    if(m_uselog)
    {
        if(m_log_mode == 1)
        {
            Log::get_instance()->init(m_log_path, m_uselog, 2000, 800000, 800);
        } else{
            Log::get_instance()->init(m_log_path, m_uselog, 2000, 800000, 0);
        }
    }
}
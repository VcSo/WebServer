#include "Server.h"

Server::Server()
{

}

Server::~Server() {}

Server::Server(int port, std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                    bool close_log, int lingermode, int et, int sql_threadnum, int threadnum, int actor_mode, int async)
                    : m_port(port), m_localhost(localhost), m_sql_username(sql_username), m_sql_password(sql_password), m_sql_database(sql_database),
                    m_close_log(close_log), m_lingermode(lingermode), m_et(et), m_sqlthreadnum(sql_threadnum), m_threadnum(threadnum), m_actor_mode(actor_mode), m_async(async)

{
    char server_path[256];
    getcwd(server_path, 256);
    char root[] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    Users = new Http[MAX_FD];

}

void Server::set_log(std::string path)
{
    if(m_close_log == true)
    {
        if(m_async == 1)
        {
            Log::get_instance()->init(path, m_close_log, 2000, 800000, 800);
        }
        else
        {
            Log::get_instance()->init(path, m_close_log, 2000, 800000, 0);
        }
    }
}

void Server::setsql()
{
    m_sql = ConnSql::getinstance();
    m_sql->init(m_localhost, m_sql_username, m_sql_password, m_sql_database, 3306, m_sqlthreadnum, m_close_log);
    Users->init_mysqlresult(m_sql);
    LOG_INFO("Test");
}

void Server::threadpool()
{
    m_pool
}

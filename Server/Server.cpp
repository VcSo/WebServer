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
    m_pool = new ThreadPool<Http>(m_actor_mode, m_sql, m_threadnum);
}

void Server::trig_mode()
{
    switch(m_et)
    {
        case 0:
            m_listen_mode = 0;
            m_conn_mode = 0;
            break;
        case 1:
            m_listen_mode = 0;
            m_conn_mode = 1;
            break;
        case 2:
            m_listen_mode = 1;
            m_conn_mode = 0;
            break;
        case 3:
            m_listen_mode = 1;
            m_conn_mode = 1;
            break;
        default:
            break;
    }
}

void Server::event_listen()
{
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    if(m_lingermode == 0)
    {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if(m_lingermode == 1)
    {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(m_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int reuse = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    ret = bind(m_listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    utils.init(TIMESLOT);

    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_listen_mode);

    Http::m_epollfd = m_epollfd;
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);

    utils.setnonblocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);
    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void Server::Start()
{
    bool timeout = true;
    bool stop_server = false;

    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);

        if(number < 0 || errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            if(sockfd == m_listenfd)
            {
                bool flag = dealclientdata();
                if(flag == false)
                {
                    continue;
                }
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
            else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN))
            {

            }
            else if (events[i].events & EPOLLIN)
            {

            }
            else if (events[i].events & EPOLLOUT)
            {

            }
        }

        if (timeout)
        {
            utils.timer_handler();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }

}

bool Server::dealclientdata()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if(m_listen_mode == 0)
    {
        int connfd = accept(m_listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
        if(connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }

        if(Http::m_user_count >= MAX_FD)
        {
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }

        timer(connfd, client_addr);
    }
}

void Server::timer(int connfd, struct sockaddr_in client_addr)
{
    Users[connfd].init(connfd, client_addr, m_root, m_conn_mode, m_close_log, m_sql_username, m_sql_password, m_sql_database);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[connfd].address = client_addr;
    users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    utils.m_timer_lst.add_timer(timer);
}

void Server::deal_timer(util_timer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if(timer)
    {
        utils.m_timer_lst.del_timer(timer);
    }
}
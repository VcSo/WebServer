#include "Server.h"

Server::Server(int port, std::string host, std::string username, std::string password, std::string database,
                int uselog, int linger, int mode, int sqlnum, int threadnum, int actor, int logmode)
                : m_port(port), m_host(host), m_username(username), m_password(password), m_database(database),
                    m_close_log(uselog), m_linger(linger), et(mode), m_sql_num(sqlnum), m_thread_num(threadnum), m_actor(actor), m_log_mode(logmode)
{
    users = new Http[MAX_FD];
    users_timer = new client_data[MAX_FD];
}

Server::~Server()
{
    delete[] users;
    delete[] users_timer;
    delete m_pool;

}

void Server::setsql()
{
    m_consql = connSql::get_instance();
    m_consql->init(m_host, m_username, m_password, m_database, m_sql_num);
    //users->initmysql_result(m_connPool);
}

void Server::setlog(std::string path)
{
    m_log_path = path;
    if(m_close_log)
    {
        if(m_log_mode == 1)
        {
            Log::get_instance()->init(m_log_path, m_close_log, 2000, 800000, 800);
        } else{
            Log::get_instance()->init(m_log_path, m_close_log, 2000, 800000, 0);
        }
    }

}

void Server::threadpool()
{
    m_pool = new ThreadPool<Http>(m_actor, m_consql, m_thread_num);
}

void Server::trig_mode()
{
    switch(et)
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
            m_listen_mode = 1;
            m_conn_mode = 1;
            break;
    }
}

void Server::event_listen()
{
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    struct linger tmp = {m_linger, 1};
    setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(m_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int reuse = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    ret = bind(m_listenfd, (struct sockaddr *) &saddr, sizeof(saddr));
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

    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void Server::Start()
{
    bool timeout = false;
    bool stop_server = false;
    LOG_INFO("%s", "Server Start");

    while(!stop_server)
    {

        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            if(sockfd == m_listenfd)
            {
                bool flag = deal_client_data();
            }

        }

    }
}

bool Server::deal_client_data()
{
    struct sockaddr_in client_addr;
    //memset(&client_addr, '\0', sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);

    if(m_listen_mode == 0)
    {
        int connfd = accept(m_listenfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if(connfd < 0)
        {
            LOG_ERROR("%s: errno is %d", "accept error", errno);
            return false;
        }

        if(Http::m_user_count >= MAX_FD)
        {
            utils.show_error(connfd, "server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        timer(connfd, client_address);
    }
    else
    {
        while(true)
        {
            int connfd = accept(m_listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
            if(connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }

            if(Http::m_user_count >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }

            timer(connfd, client_addr);
        }
    }

}

void Server::timer(int connfd, struct sockaddr_in client_addr)
{
    users
}
#include "Server.h"

Server::Server()
{

}

Server::~Server() {
//    delete m_pool;
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete []Users;
    delete []users_timer;
    free(m_root);
    free(m_down);

}

Server::Server(std::string ip, int port, std::string localhost, std::string sql_username, std::string sql_password, std::string sql_database,
                    bool close_log, int lingermode, int et, int sql_threadnum, int threadnum, int actor_mode, int async)
                    : m_ip(ip), m_port(port), m_localhost(localhost), m_sql_username(sql_username), m_sql_password(sql_password), m_sql_database(sql_database),
                    m_close_log(close_log), m_lingermode(lingermode), m_et(et), m_sqlthreadnum(sql_threadnum), m_threadnum(threadnum), m_actor_mode(actor_mode), m_async(async)

{
    Users = new Http[MAX_FD];
    users_timer = new client_data[MAX_FD];

    char server_path[256];
    getcwd(server_path, 256);

    char root[] = "/resources";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root); //拼接

    char down_dir[] = "/SaveFile";
    m_down = (char *)malloc(strlen(server_path) + strlen(down_dir) + 1);
    strcpy(m_down, server_path);
    strcat(m_down, down_dir); //拼接

}

void Server::set_log(const std::string path)
{
    if(m_close_log == true)
    {
        if(m_async == 1)
        {
            Log::get_instance()->init(path, m_close_log, 2000, 1000000, 800);
            LOG_INFO("LOG Test");
        }
        else
        {
            Log::get_instance()->init(path, m_close_log, 2000, 800000, 0);
            LOG_INFO("LOG Test");
        }

        std::cout << "Log init" << std::endl;
    }
}

void Server::set_savefile(const std::string filepath)
{
    if(access(filepath.c_str(), F_OK) == -1 ){// 文件夹不存在, https://www.cnblogs.com/whwywzhj/p/7801409.html
        mkdir(filepath.c_str(), S_IRWXO|S_IRWXG|S_IRWXU);// https://www.jianshu.com/p/06a0da1f6389
    }
}

void Server::setsql()
{
    m_sql = ConnSql::getinstance();
    m_sql->init(m_localhost, m_sql_username, m_sql_password, m_sql_database, 3306, m_sqlthreadnum, m_close_log);
    Users->init_mysqlresult(m_sql);
    std::cout << "Connected Mysql" << std::endl;
    LOG_INFO("SQL Connected");
}

void Server::threadpool()
{
//    m_pool = new ThreadPool<Http>(m_actor_mode, m_sql, m_threadnum);
    m_pool = std::unique_ptr<ThreadPool<Http>>(new ThreadPool<Http>(m_actor_mode, m_sql, m_threadnum)); //独占
    std::cout << "ThreadPool init" << std::endl;
    LOG_INFO("ThreadPool created")
}


void Server::trig_mode()
{
    switch(m_et)
    {
        case 0: //LT+LT
            m_listen_mode = 0;
            m_conn_mode = 0;
            break;
        case 1: //LT+ET
            m_listen_mode = 0;
            m_conn_mode = 1;
            break;
        case 2: //ET+LT
            m_listen_mode = 1;
            m_conn_mode = 0;
            break;
        case 3: //ET+ET
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

    /*
     * struct linger {
     *  int l_onoff; 0 = off, nozero = on
     *  int l_linger; linger time
     * };
     */
    struct linger tmp = {m_lingermode, 1}; /*  lingmode = 0，优雅关闭: 直到所剩数据发送完毕或超时,正常4次挥手 */
    setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(m_port);
    if(m_ip == "")
    {
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        address.sin_addr.s_addr = inet_addr(m_ip.c_str());
    }

    int reuse = 1;
    if(setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        LOG_ERROR("setsockopt SO_REUSEADDR error");
    }
    //https://www.zhihu.com/question/42308970/answer/246334766
    if(setsockopt(m_listenfd, IPPROTO_TCP, TCP_NODELAY, (char*)&reuse, int(sizeof(int))) == -1)
    {
        LOG_ERROR("setsockopt TCP_NODELAY error");
    }

    ret = bind(m_listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    utils.init(TIMESLOT);

//    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_listen_mode);

    Http::m_epollfd = m_epollfd;
    //创建管道套接字
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);

    //设置管道写端为非阻塞
    utils.setnonblocking(m_pipefd[1]);
    //设置管道读端为ET非阻塞 统一事件源
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);

    //传递给主循环的信号值，只关注SIGALRM和SIGTERM
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void Server::Start()
{
    bool timeout = false;
    bool stop_server = false;
    std::cout << "Server start" << std::endl;

    while(!stop_server)
    {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);

        if(number < 0 && (errno != EINTR))
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;

            if(sockfd == m_listenfd) //新连接
            {
//                LOG_INFO("%s", "dealclientdata");
                bool flag = dealclientdata();
                if(flag == false)
                {
                    continue;
                }
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) //异常事件处理，关闭连接，移除定时器
            {
//                LOG_INFO("%s", "deal_timer");
//                std::cout << "deal_timer()" << std::endl;
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
            else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN)) //处理信号
            {
//                LOG_INFO("%s", "dealwithsignal");
                bool flag = dealwithsignal(timeout, stop_server);
                if (flag == false)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            else if (events[i].events & EPOLLIN)
            {
//                LOG_INFO("%s", "dealwithread");
                dealwithread(sockfd);
            }
            else if (events[i].events & EPOLLOUT)
            {
//                LOG_INFO("%s", "dealwithwrite");
                dealwithwrite(sockfd);
            }
        }

        if(timeout)
        {
            utils.timer_handler();
//            LOG_INFO("%s", "timer tick");
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
    else
    {
        while(true)
        {
            int connfd = accept(m_listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
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
        return false;
    }
    return true;
}

void Server::timer(int connfd, struct sockaddr_in client_addr)
{
    Users[connfd].init(connfd, client_addr, m_root, m_down,m_conn_mode, m_close_log, m_sql_username, m_sql_password, m_sql_database);

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
    utils.m_timer_list.add_timer(timer);
}

void Server::deal_timer(util_timer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if(timer)
    {
        utils.m_timer_list.del_timer(timer);
    }

    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}

bool Server::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig = 0;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else
    {
        for(int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM: //时种定时 errno 14
                {
                    timeout = true;
                    break;
                }
                case SIGTERM: //程序结束运行
                {
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;

}

void Server::dealwithread(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    //reactor
    if(m_actor_mode == 1)
    {
        if(timer)
        {
            adjust_timer(timer);
        }

        m_pool->append(Users + sockfd, 0);

        while(true)
        {
            if (1 == Users[sockfd].improv)
            {
                if (1 == Users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    Users[sockfd].timer_flag = 0;
                }
                Users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
        //proactor
        if (Users[sockfd].read_once())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(Users[sockfd].get_address()->sin_addr));

            //若监测到读事件，将该事件放入请求队列
            m_pool->append_p(Users + sockfd);

            if(timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}

void Server::adjust_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 + TIMESLOT;
    utils.m_timer_list.adjust_timer(timer);

    LOG_INFO("%s", "adjust timer once");
}

void Server::dealwithwrite(int sockfd)
{
    util_timer *timer = users_timer[sockfd].timer;

    if (1 == m_actor_mode)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        m_pool->append(Users + sockfd, 1);

        while(true)
        {
            if(1 == Users[sockfd].improv)
            {
                if(1 == Users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    Users[sockfd].timer_flag = 0;
                }
                Users[sockfd].improv = 0;
                break;
            }
        }
    }
    else
    {
//        m_pool->append(Users + sockfd, 1);
        //proactor
        if (Users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(Users[sockfd].get_address()->sin_addr));

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            deal_timer(timer, sockfd);
        }
    }
}
#include "Server.h"

WebServer::WebServer(int port, int et, int timeout, bool optlinger,
          int sql_port, std::string sql_username, std::string sql_user_password, std::string sql_database,
          int sqlconn_num, int thread_num, bool log_flag, int log_level, int log_queue_size)
          : m_port(port), m_et(et), m_timeout(timeout), m_optlinger(optlinger),
            m_sql_port(sql_port), m_sql_username(sql_username), m_sql_password(sql_user_password), m_sql_database(sql_database),
            m_sql_conn_num(sqlconn_num), m_threadpool(new ThreadPool<WebServer>(thread_num)), m_timer(new Timer()), m_epoll(new Epoll())
{
    srcdir = getcwd(nullptr, 256);
    assert(srcdir);
    strncat(srcdir,  "/resources/", 16);
    std::cout << srcdir << std::endl;

    Httpconn::srcDir = srcdir;
    Httpconn::userCount = 0;

    SqlConnPool::Instance()->init("localhost", m_sql_port, m_sql_username, m_sql_password, m_sql_database, m_sql_conn_num);
    InitEventMode(et);
    if(!InitSocket())
    {
        isClose = true;
    }
    //if(openlog)
    {

    }

}

void WebServer::InitEventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
        case 0:
            break;
        case 1:
            connEvent |= EPOLLET;
            break;
        case 2:
            listenEvent |= EPOLLET;
            break;
        case 3:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
        default:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
    }
    Httpconn::isET = (connEvent & EPOLLET);
}

bool WebServer::InitSocket()
{
    int ret;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct linger optLinger = {0};
    if(m_optlinger)
    {
        //优雅关闭
        optLinger.l_linger = 1;
        optLinger.l_onoff = 1;
    }

    if(listenfd < 0)
    {
        //log
        std::cout << "init sockfd error" << std::endl;
        close(listenfd);
        return false;
    }

    int reuse = 1;
    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*) &reuse, sizeof(reuse));
    if(ret == -1)
    {
        //log
        close(listenfd);
        return false;
    }

    ret = bind(listenfd, (struct sockaddr*) &addr, sizeof(addr));
    if(ret < 0)
    {
        //log
        std::cout << "sockfd bind error, port = " << m_port << std::endl;
        close(listenfd);
        return false;
    }

    ret = listen(listenfd, 10);
    if(ret < 0)
    {
        //log
        std::cout << "listen sockfd error, port = " << m_port << std::endl;
        close(listenfd);
        return false;
    }

    ret = epoller->AddFd(listenfd, listenEvent | EPOLLIN);
    if(ret == 0)
    {
        //log
        close(listenfd);
        return false;
    }

    setNonblock(listenfd);
}

void WebServer::setNonblock(int fd)
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}



void WebServer::Start()
{
    int timeMS = -1;
    if(!isClose)
    {
        //LOG_INFO("========== Server start ==========");
    }

    while(!isClose)
    {
        if(timeMS > 0)
        {
            timeMS = timer->GetNextTick();
        }
        int eventCnt = epoller->Wait(timeMS);

        for(int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = epoller->GetEventFd(i);
            uint32_t events = epoller->GetEvents(i);
            if(fd == listenfd) {
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            } else {
                //log
                //LOG_ERROR("Unexpected event");
            }
        }
    }
}
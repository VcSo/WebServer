#include "Httpconn.h"

int Http::m_user_count = 0;
int Http::m_epollfd = -1;

Http::Http()
{

}

Http::~Http()
{

}

void Http::init()
{
    mysql = nullptr;
    bytes_have_end = 0;
    bytes_to_end = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = nullptr;
    m_version = nullptr;
    m_content_length = 0;
    m_host = nullptr;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

//对文件描述符设置非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Http::init(int sockfd, const struct sockaddr_in &addr, char *m_root, int conn_mode, int close_log,
                    std::string username, std::string password, std::string database)
{
    m_sockfd = sockfd;
    m_addr = addr;
    addfd(m_epollfd, sockfd, true, conn_mode);
    ++m_user_count;

    doc_root = m_root;
    m_conn_mode = conn_mode;
    m_close_log = close_log;

    sql_username = username;
    sql_password = password;
    sql_database = database;

    init();
}
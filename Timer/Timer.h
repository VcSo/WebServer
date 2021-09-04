#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include <unistd.h>
#include <cassert>
#include <cstring>
#include <time.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../Http/Httpconn.h"
#include "../Log/Log.h"

class util_timer;

struct client_data
{
    sockaddr_in addr;
    int fd;
    util_timer *timer;
};

class util_timer {
public:
    util_timer();

public:
    void (* cb_func)(client_data *);

    time_t expire;
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void tick();

private:
    void add_timer(util_timer *timer, util_timer *head);

    util_timer *head;
    util_timer *tail;
};

class Utils
{
public:
    Utils();
    ~Utils();

    void init(int timeslot);
    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);
    void addsig(int sig, void(handler)(int), bool restart = true);
    void timer_handler();
    void show_error(int connfd, const char *info);
    static void sig_handler(int sig);

    int setnonblocking(int fd);

    static int *u_pipefd;
    static int u_epollfd;
    int m_timeslot;
    sort_timer_lst m_timer_lst;
};

void cb_func(client_data *user_data);

#endif //WEBSERVER_TIMER_H

#include "Timer.h"

util_timer::util_timer() : prev(nullptr), next(nullptr)
{
}

sort_timer_lst::sort_timer_lst()
{
    head = nullptr;
    tail = nullptr;
}

sort_timer_lst::~sort_timer_lst()
{
    util_timer *tmp = head;
    while(tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_lst::del_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }
    if ((timer == head) && (timer == tail))
    {
        delete timer;
        head = nullptr;
        tail = nullptr;
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void sort_timer_lst::add_timer(util_timer *timer)
{
    if(!timer)
    {
        return;
    }

    if(!head)
    {
        head = tail = timer;
        return;
    }

    if(timer->expire < head->expire)
    {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }

    add_timer(timer, head);
}

void sort_timer_lst::add_timer(util_timer *timer, util_timer *head)
{
    util_timer *tmp = timer;
    util_timer *headtimer = head;

    while(tmp)
    {
        if(tmp->expire < headtimer->expire)
        {
            timer->next = headtimer;
            headtimer->prev = timer;
            headtimer = timer;
            break;
        }

        headtimer = tmp;
        tmp = tmp->next;
    }

    if(!tmp)
    {
        headtimer->next = timer;
        timer->prev = headtimer;
        timer->next = nullptr;
        tail = timer;
    }

}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

Utils::Utils() {}

Utils::~Utils() {}

void Utils::init(int timeslot)
{
    m_timeslot = timeslot;
}

int Utils::setnonblocking(int fd)
{
    int oldoption = fcntl(fd, F_GETFL);
    int newoption = oldoption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newoption);
    return oldoption;
}

void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;
    if(TRIGMode == 1)
    {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    else
    {
        event.events = EPOLLIN | EPOLLRDHUP;
    }

    if(one_shot)
    {
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Utils::sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}


void Utils::addsig(int sig, void (*handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;

    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->fd, 0);
    assert(user_data);
    close(user_data->fd);
    Http::m_user_count--;
}
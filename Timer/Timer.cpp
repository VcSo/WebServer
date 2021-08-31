#include "Timer.h"

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
    memset(&sa. '\0', sizeof(sa));
    sa.sa_handler = handler;

    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}
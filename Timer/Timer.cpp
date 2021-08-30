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
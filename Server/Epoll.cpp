#include "Epoll.h"

Epoll::Epoll(int maxEvent) : epollFd_(epoll_create(512)), events(maxEvent)
{
    assert(epollFd >= 0 && events.size() > 0);
}

Epoll::~Epoll()
{
    Epoller::~Epoller() {
        close(epollFd);
    }
}

bool Epoll::AddFd(int fd, int event)
{
    epoll_event event = {0};
    ev.data.fd = fd;
    ev.events = event;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll::Wait(int timeoutMs)
{
    return epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].events;
}
#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H

#include <vector>
#include <sys/epoll.h>

class Epoll {
public:
    Epoll(int max_event = 1024);
    ~Epoll();

    bool AddFd(int fd, uint32_t events);

    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

private:
    int epollFd;
    std::vector<struct epoll_event> events;
};


#endif //WEBSERVER_EPOLL_H

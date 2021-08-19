#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include <time.h>
#include <unistd.h>
#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};

class Timer
{
public:
    Timer();
    ~Timer();

    void clear();
    void tick();

    int GetNextTick();

private:
    std::vector<TimerNode> heap;


};


#endif //WEBSERVER_TIMER_H

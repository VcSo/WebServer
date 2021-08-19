#include "Timer.h"

Timer::Timer()
{

}

Timer::~Timer()
{

}

int Timer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap.empty()) {
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}

void Timer::tick()
{
    if(heap.empty())
    {
        return;
    }

    while(!heap.empty())
    {
        TimerNode node = head.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }

        node.cb();
        pop();
    }
}
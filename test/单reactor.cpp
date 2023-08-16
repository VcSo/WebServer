//
// Created by Vc on 2023/8/15.
//

while(!stop)
{
    // 1.取得下次定时任务的时间，与设定time_out去较大值，即若下次定时任务时间超过1s就取下次定时任务时间为超时时间，否则取1s
    int time_out = Max(1000, getNextTimerCallback());
    // 2.调用Epoll等待事件发生，超时时间为上述的time_out
    int rt = epoll_wait(epfd, fds, ...., time_out);
    if(rt < 0) {
    // epoll调用失败。。
    } else {
        if(rt > 0)
        {

        }
    }

}
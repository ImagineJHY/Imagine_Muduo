#ifndef IMAGINE_MUDUO_EVENTLOOP_H
#define IMAGINE_MUDUO_EVENTLOOP_H

#include <pthread.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <functional>
#include <queue>
#include <memory>
#include <unordered_map>

#include "TimeUtil.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "TimeStamp.h"
#include "Callbacks.h"

namespace Imagine_Muduo
{

class Channel;

class Poller;
class EpollPoller;

class EventLoop
{
 public:
    EventLoop(int port, int max_channel = 10000, EventCallback read_cb = nullptr, EventCallback write_cb = nullptr, EventCommunicateCallback communicate_cb = nullptr);

    ~EventLoop();

    void loop();

    int GetChannelnum();

    void AddChannelnum();

    int GetMaxchannelnum();

    Poller *GetEpoll();

    EventCallback GetReadCallback();

    EventCallback GetWriteCallback();

    EventCommunicateCallback GetCommunicateCallback();

    void SetReadCallback(EventCallback read_callback_);

    void SetWriteCallback(EventCallback write_callback_);

    void SetCommunicateCallback(EventCommunicateCallback communicate_callback);

    void UpdateChannel(std::shared_ptr<Channel> channel);

    void Close(std::shared_ptr<Channel> channel);

    void Closefd(int fd); // 心跳检测根据fd删除channel用

    void DestroyClosedChannel();

    /*
    -所有的时间参数都是以秒为单位，并且是从loop开始的相对时间
    -delay_表示首次开始执行的延时时间,为0表示从当前开始每interval_时间执行一次
    -interval_表示每次执行的间隔时间,为0表示只执行一次
    -delay_与interval_不能同时为0
    */
    long long SetTimer(TimerCallback timer_callback, double interval, double delay = 0.0);

    bool CloseTimer(long long timer_id);

    bool InsertTimer(Timer *timer);

    std::vector<Timer *> GetExpiredTimers(const TimeStamp &now);

    // std::shared_ptr<Channel> GetTimer(){return timer_channel;}

 private:
    int quit_;
    ThreadPool<std::shared_ptr<Channel>> *pool_;
    std::list<std::shared_ptr<Channel>> close_list_;
    pthread_t *destroy_thread_;
    pthread_mutex_t destroy_lock_;
    int channel_num_;           // 当前连接的客户端数目
    int max_channel_num_;       // 最多能接收的客户端数目
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCommunicateCallback communicate_callback_ = nullptr;
    Poller *epoll_;
    // vector<Channel*> channels_;
    std::shared_ptr<Channel> listen_channel_;

    pthread_mutex_t timer_lock_;
    std::shared_ptr<Channel> timer_channel_;
    std::priority_queue<Timer *, std::vector<Timer *>, TimeUtil::TimerPtrCmp> timers_;

    pthread_mutex_t timer_map_lock_;
    std::unordered_map<long long, Timer *> timer_map_;
    // const int timerfd_;
};

} // namespace Imagine_Muduo

#endif
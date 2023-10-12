#include "EventLoop.h"

#include <memory>

#include "Channel.h"
#include "Poller.h"
#include "ThreadPool.h"

using namespace Imagine_Muduo;

long long Timer::id_ = 0;

EventLoop::EventLoop(int port, int max_channel, EventCallback read_cb, EventCallback write_cb, EventCommunicateCallback communicate_cb)
                 : quit_(0), channel_num_(0), max_channel_num_(max_channel), read_callback_(read_cb), write_callback_(write_cb), communicate_callback_(communicate_cb), epoll_(new EpollPoller(this)), listen_channel_(Channel::Create(this, port, 0)), timer_channel_(Channel::Create(this, 0, 2))
{
    try {
        pool_ = new ThreadPool<std::shared_ptr<Channel>>(10, max_channel); // 初始化线程池
    } catch (...) {
        throw std::exception();
    }

    destroy_thread_ = new pthread_t;
    if (!destroy_thread_) {
        throw std::exception();
    }

    if (pthread_mutex_init(&destroy_lock_, nullptr) != 0) {
        throw std::exception();
    }

    if (pthread_mutex_init(&timer_lock_, nullptr) != 0) {
        throw std::exception();
    }

    if (pthread_mutex_init(&timer_map_lock_, nullptr) != 0) {
        throw std::exception();
    }

    epoll_->AddChannel(timer_channel_);
    epoll_->AddChannel(listen_channel_); // 创建监听套接字并添加到epoll
}

EventLoop::~EventLoop()
{
    delete pool_;
    delete epoll_;
}

void EventLoop::loop()
{
    if (pthread_create(
            destroy_thread_, nullptr, [](void *argv) -> void *
            {
                EventLoop* loop = (EventLoop*)argv;
                while(1){
                    loop->DestroyClosedChannel();
                }

                return nullptr;
            },
            this) != 0) {
        delete[] destroy_thread_;
        printf("loop exception!\n");
        throw std::exception();
    }

    if (pthread_detach(*destroy_thread_)) {
        delete[] destroy_thread_;
        throw std::exception();
    }

    // epoll->AddChannel(listen_channel);
    // epoll->AddChannel(timer_channel);

    while (!quit_) {
        std::vector<std::shared_ptr<Channel>> active_channels;
        epoll_->poll(-1, &active_channels);
        while (active_channels.size()) {
            pool_->PutTask(active_channels[active_channels.size() - 1]);
            active_channels.pop_back();
        }
    }
}

int EventLoop::GetChannelnum()
{
    return channel_num_;
}

void EventLoop::AddChannelnum()
{
    channel_num_++;
}

int EventLoop::GetMaxchannelnum()
{
    return max_channel_num_;
}

Poller *EventLoop::GetEpoll()
{
    return epoll_;
}

EventCallback EventLoop::GetReadCallback()
{
    return read_callback_;
}

EventCallback EventLoop::GetWriteCallback()
{
    return write_callback_;
}

EventCommunicateCallback EventLoop::GetCommunicateCallback()
{
    return communicate_callback_;
}

void EventLoop::SetReadCallback(EventCallback read_callback)
{
    read_callback_ = read_callback;
}

void EventLoop::SetWriteCallback(EventCallback write_callback)
{
    write_callback_ = write_callback;
}

void EventLoop::SetCommunicateCallback(EventCommunicateCallback communicate_callback)
{
    communicate_callback_ = communicate_callback;
}

void EventLoop::UpdateChannel(std::shared_ptr<Channel> channel)
{
    epoll_->Update(channel->Getfd(), channel->GetEvents());
}

void EventLoop::Close(std::shared_ptr<Channel> channel)
{
    if (channel.get() == nullptr)
        return;
    channel_num_--;
    epoll_->DelChannel(channel);
    pthread_mutex_lock(&destroy_lock_);
    close_list_.push_back(channel);
    pthread_mutex_unlock(&destroy_lock_);
}

void EventLoop::Closefd(int fd)
{
    Close(epoll_->FindChannel(fd));
}

void EventLoop::DestroyClosedChannel()
{
    while (close_list_.size()) {
        pthread_mutex_lock(&destroy_lock_);
        // printf("im destroyChannel!\n");
        Channel::Destroy(close_list_.back());
        close_list_.back().reset();
        close_list_.pop_back();
        pthread_mutex_unlock(&destroy_lock_);
    }
}

long long EventLoop::SetTimer(TimerCallback timer_callback, double interval, double delay)
{
    if (interval == 0 && delay == 0) {
        return false;
    }
    TimeStamp new_time;
    if (delay) {
        new_time.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(), delay));
    } else {
        new_time.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(), interval));
    }
    Timer *new_timer = new Timer(new_time, timer_callback, interval);
    // printf("now is %lld\nnew is %lld\n",TimeUtil::GetNow(),new_timer->GetCallTime().GetTime());
    InsertTimer(new_timer);

    return new_timer->GetTimerId();
}

bool EventLoop::CloseTimer(long long timer_id)
{
    pthread_mutex_lock(&timer_map_lock_);
    // printf("timerid is %lld\n",timer_id);
    std::unordered_map<long long, Timer *>::iterator it = timer_map_.find(timer_id);
    if (it == timer_map_.end()) {
        throw std::exception();
    }
    it->second->Close();
    timer_map_.erase(it);
    pthread_mutex_unlock(&timer_map_lock_);

    return true;
}

bool EventLoop::InsertTimer(Timer *timer)
{
    pthread_mutex_lock(&timer_lock_);
    timers_.push(timer);
    if (timers_.top() == timer) {
        // printf("now is %lld\nnew is %lld\n",TimeUtil::GetNow(),timers_.top()->GetCallTime().GetTime());
        TimeUtil::ResetTimerfd(timer_channel_->Getfd(), timers_.top()->GetCallTime());
    }
    pthread_mutex_lock(&timer_map_lock_);
    timer_map_.insert(std::make_pair(timer->GetTimerId(), timer));
    pthread_mutex_unlock(&timer_map_lock_);
    pthread_mutex_unlock(&timer_lock_);

    return true;
}

std::vector<Timer *> EventLoop::GetExpiredTimers(const TimeStamp &now)
{
    std::vector<Timer *> expired_timers;
    pthread_mutex_lock(&timer_lock_);
    Timer *top_timer = timers_.top();
    // 比较绝对时间
    while (timers_.size() && (top_timer->GetCallTime().GetTime()) < (now.GetTime())) {
        expired_timers.push_back(top_timer);
        timers_.pop();
        top_timer = timers_.top();
    }
    if (timers_.size()) {
        TimeUtil::ResetTimerfd(timer_channel_->Getfd(), timers_.top()->GetCallTime());
    } else {
        TimeUtil::SetDefaultTimerfd(timer_channel_->Getfd());
    }
    pthread_mutex_unlock(&timer_lock_);

    return expired_timers;
}
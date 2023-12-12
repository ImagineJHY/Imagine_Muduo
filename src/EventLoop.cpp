#include "Imagine_Muduo/EventLoop.h"

#include "Imagine_Muduo/Channel.h"
#include "Imagine_Muduo/EpollPoller.h"
#include "Imagine_Muduo/ThreadPool.h"

#include <memory>
#include <fstream>

namespace Imagine_Muduo
{

EventLoop::EventLoop()
            : quit_(0), channel_num_(0), epoll_(new EpollPoller(this)), timer_channel_(Channel::Create(this, 0, Channel::ChannelTyep::TimerChannel))
{
}

EventLoop::EventLoop(const std::string& profile_name) : EventLoop()
{
    Init(profile_name);
}

EventLoop::EventLoop(const YAML::Node& config) : EventLoop()
{
    Init(config);
}

EventLoop::~EventLoop()
{
    delete thread_pool_;
    delete epoll_;
}

void EventLoop::Init(const std::string& profile_name)
{
    if (profile_name == "") {
        throw std::exception();
    }
    YAML::Node config = YAML::LoadFile(profile_name);
    Init(config);
}

void EventLoop::Init(const YAML::Node& config)
{
    port_ = config["port"].as<size_t>();
    thread_num_ = config["thread_num"].as<size_t>();
    max_channel_num_ = config["max_channel_num"].as<size_t>();
    singleton_log_mode_ = config["singleton_log_mode"].as<bool>();

    if (singleton_log_mode_) {
        logger_ = SingletonLogger::GetInstance();
    } else {
        logger_ = new NonSingletonLogger();
        Logger::SetInstance(logger_);
    }

    logger_->Init(config);

    InitLoop();
}

void EventLoop::InitLoop()
{
    listen_channel_ = Channel::Create(this, port_, Channel::ChannelTyep::ListenChannel);

    try {
        thread_pool_ = new ThreadPool<std::shared_ptr<Channel>>(thread_num_, max_channel_num_); // 初始化线程池
    } catch (...) {
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

void EventLoop::loop()
{
    while (!quit_) {
        std::vector<std::shared_ptr<Channel>> active_channels;
        epoll_->poll(-1, active_channels);
        while (active_channels.size()) {
            thread_pool_->PutTask(active_channels[active_channels.size() - 1]);
            active_channels.pop_back();
        }
    }
}

int EventLoop::GetChannelnum() const
{
    return channel_num_;
}

EventLoop* EventLoop::AddListenChannel(const std::string& port)
{
    Channel::Create(this, atoi(&port[0]), Channel::ChannelTyep::ListenChannel);

    return this;
}

EventLoop* EventLoop::AddListenChannel(int port)
{
    Channel::Create(this, port, Channel::ChannelTyep::ListenChannel);

    return this;
}

EventLoop* EventLoop::AddEventChannel(const std::string& port)
{
    Channel::Create(this, atoi(&port[0]), Channel::ChannelTyep::EventChannel);

    return this;
}

EventLoop* EventLoop::AddEventChannel(int port)
{
    Channel::Create(this, port, Channel::ChannelTyep::EventChannel);

    return this;
}

std::shared_ptr<Channel> EventLoop::GetListenChannel() const
{
    return listen_channel_;
}

int EventLoop::GetMaxchannelnum() const
{
    return max_channel_num_;
}

 EventLoop* EventLoop::AddChannel(std::shared_ptr<Channel> channel)
 {
    epoll_->AddChannel(channel);
    channel_num_++;

    return this;
 }

const EventLoop* EventLoop::UpdateChannel(std::shared_ptr<Channel> channel) const
{
    epoll_->Update(channel->Getfd(), channel->GetEvents());

    return this;
}

EventLoop* EventLoop::CloseChannel(std::shared_ptr<Channel> channel)
{
    if (channel.get() == nullptr) {
        return this;
    }
    channel_num_--;
    epoll_->DelChannel(channel);

    return this;
}

EventLoop* EventLoop::Closefd(int fd)
{
    CloseChannel(epoll_->FindChannel(fd));

    return this;
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
    InsertTimer(new_timer);

    return new_timer->GetTimerId();
}

EventLoop* EventLoop::CloseTimer(long long timer_id)
{
    pthread_mutex_lock(&timer_map_lock_);
    std::unordered_map<long long, Timer *>::iterator it = timer_map_.find(timer_id);
    if (it == timer_map_.end()) {
        pthread_mutex_unlock(&timer_map_lock_);
        return this;
    }
    it->second->Close();
    timer_map_.erase(it);

    return this;
}

EventLoop* EventLoop::InsertTimer(Timer *timer)
{
    pthread_mutex_lock(&timer_lock_);
    timers_.push(timer);
    if (timers_.top() == timer) {
        TimeUtil::ResetTimerfd(timer_channel_->Getfd(), timers_.top()->GetCallTime());
    }
    pthread_mutex_lock(&timer_map_lock_);
    if (timer_map_.find(timer->GetTimerId()) == timer_map_.end()) {
        timer_map_.insert(std::make_pair(timer->GetTimerId(), timer));
    }
    pthread_mutex_unlock(&timer_map_lock_);
    pthread_mutex_unlock(&timer_lock_);

    return this;
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

} // namespace Imagine_Muduo
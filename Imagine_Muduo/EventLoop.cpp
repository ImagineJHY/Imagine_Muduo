#include"EventLoop.h"

#include<memory>

#include"Channel.h"
#include"Poller.h"
#include"ThreadPool.h"

using namespace Imagine_Muduo;

long long Timer::id=0;

EventLoop::EventLoop(int port,int max_channel, EventCallback read_cb, EventCallback write_cb, EventCommunicateCallback communicate_cb):
        max_channel_num(max_channel),channel_num(0),quit(0),read_callback(read_cb),write_callback(write_cb),communicate_callback(communicate_cb),epoll(new EpollPoller(this)),timer_channel(Channel::Create(this,0,2)),listen_channel(Channel::Create(this,port,0))
{
        try{
            pool=new ThreadPool<std::shared_ptr<Channel>>(10,max_channel);//初始化线程池
        }catch(...){
            throw std::exception();
        }

        destroy_thread=new pthread_t;
        if(!destroy_thread){
            throw std::exception();
        }

        if(pthread_mutex_init(&destroy_lock,nullptr)!=0){
            throw std::exception();
        }

        if(pthread_mutex_init(&timer_lock,nullptr)!=0){
            throw std::exception();
        }

        if(pthread_mutex_init(&timer_map_lock,nullptr)!=0){
            throw std::exception();
        }

        epoll->AddChannel(timer_channel);
        epoll->AddChannel(listen_channel);//创建监听套接字并添加到epoll
    }

EventLoop::~EventLoop(){
    delete pool;
    delete epoll;
}

void EventLoop::loop(){

    if(pthread_create(destroy_thread,nullptr,[](void* argv)->void*{
        EventLoop* loop=(EventLoop*)argv;
        while(1){
            loop->DestroyClosedChannel();
        }

        return nullptr;
    },this)!=0){
        delete [] destroy_thread;
        printf("loop exception!\n");
        throw std::exception();
    }

    if(pthread_detach(*destroy_thread)){
        delete [] destroy_thread;
        throw std::exception();
    }

    // epoll->AddChannel(listen_channel);
    // epoll->AddChannel(timer_channel);

    while(!quit){
        std::vector<std::shared_ptr<Channel>> active_channels;
        epoll->poll(-1,&active_channels);
        while (active_channels.size()){
            pool->PutTask(active_channels[active_channels.size()-1]);
            active_channels.pop_back();
        }
    }
}

int EventLoop::GetChannelnum(){
    return channel_num;
}

void EventLoop::AddChannelnum(){
    channel_num++;
}

int EventLoop::GetMaxchannelnum(){
    return max_channel_num;
}

Poller* EventLoop::GetEpoll(){
    return epoll;
}

EventCallback EventLoop::GetReadCallback(){
    return read_callback;
}

EventCallback EventLoop::GetWriteCallback(){
        return write_callback;
}

EventCommunicateCallback EventLoop::GetCommunicateCallback(){
    return communicate_callback;
}

void EventLoop::SetReadCallback(EventCallback read_callback_){
    read_callback=read_callback_;
}

void EventLoop::SetWriteCallback(EventCallback write_callback_){
    write_callback=write_callback_;
}

void EventLoop::SetCommunicateCallback(EventCommunicateCallback communicate_callback_){
    communicate_callback=communicate_callback_;
}

void EventLoop::UpdateChannel(std::shared_ptr<Channel> channel){
    epoll->Update(channel->Getfd(),channel->GetEvents());
}

void EventLoop::Close(std::shared_ptr<Channel> channel)
{
    if(channel.get()==nullptr)return;
    channel_num--;
    epoll->DelChannel(channel);
    pthread_mutex_lock(&destroy_lock);
    close_list.push_back(channel);
    pthread_mutex_unlock(&destroy_lock);
}

void EventLoop::Closefd(int fd)
{
    Close(epoll->FindChannel(fd));
}

void EventLoop::DestroyClosedChannel()
{
    while(close_list.size()){
        pthread_mutex_lock(&destroy_lock);
        //printf("im destroyChannel!\n");
        Channel::Destroy(close_list.back());
        close_list.back().reset();
        close_list.pop_back();
        pthread_mutex_unlock(&destroy_lock);
    }
}

long long EventLoop::SetTimer(TimerCallback timer_callback_, double interval_, double delay_)
{
    if(interval_==0&&delay_==0)return false;
    TimeStamp new_time;
    if(delay_){
        new_time.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(),delay_));
    }else{
        new_time.SetTime(TimeUtil::MicroSecondsAddSeconds(TimeUtil::GetNow(),interval_));
    }
    Timer* new_timer=new Timer(new_time,timer_callback_,interval_);
    //printf("now is %lld\nnew is %lld\n",TimeUtil::GetNow(),new_timer->GetCallTime().GetTime());
    InsertTimer(new_timer);

    return new_timer->GetTimerId();
}

bool EventLoop::CloseTimer(long long timer_id)
{
    pthread_mutex_lock(&timer_map_lock);
    // printf("timerid is %lld\n",timer_id);
    std::unordered_map<long long,Timer*>::iterator it=timer_map.find(timer_id);
    if(it==timer_map.end()){
        throw std::exception();
    }
    it->second->Close();
    timer_map.erase(it);
    pthread_mutex_unlock(&timer_map_lock);

    return true;
}

bool EventLoop::InsertTimer(Timer* timer_)
{
    pthread_mutex_lock(&timer_lock);
    timers_.push(timer_);
    if(timers_.top()==timer_){
        //printf("now is %lld\nnew is %lld\n",TimeUtil::GetNow(),timers_.top()->GetCallTime().GetTime());
        TimeUtil::ResetTimerfd(timer_channel->Getfd(),timers_.top()->GetCallTime());
    }
    pthread_mutex_lock(&timer_map_lock);
    timer_map.insert(std::make_pair(timer_->GetTimerId(),timer_));
    pthread_mutex_unlock(&timer_map_lock);
    pthread_mutex_unlock(&timer_lock);

    return true;
}

std::vector<Timer*> EventLoop::GetExpiredTimers(const TimeStamp& now_)
{
    std::vector<Timer*> expired_timers;
    pthread_mutex_lock(&timer_lock);
    Timer* top_timer=timers_.top();
    //比较绝对时间
    while(timers_.size()&&(top_timer->GetCallTime().GetTime())<(now_.GetTime())){
        expired_timers.push_back(top_timer);
        timers_.pop();
        top_timer=timers_.top();
    }
    if(timers_.size())TimeUtil::ResetTimerfd(timer_channel->Getfd(),timers_.top()->GetCallTime());
    else TimeUtil::SetDefaultTimerfd(timer_channel->Getfd());
    pthread_mutex_unlock(&timer_lock);

    return expired_timers;
}
#include"Poller.h"

#include<memory>

#include"EventLoop.h"
#include"Channel.h"

using namespace Imagine_Muduo;

EpollPoller::EpollPoller(EventLoop* loop_){
        loop=loop_;
        channel_num=0;
        epollfd=epoll_create(100);

        if(pthread_mutex_init(&hashmap_lock,nullptr)!=0){
            throw std::exception();
        }
}

void EpollPoller::poll(int timeoutMs, std::vector<std::shared_ptr<Channel>>* active_channels){
    
    epoll_event* events_set=new epoll_event[channel_num];
    int events_num=epoll_wait(epollfd,events_set,channel_num,timeoutMs);
    //printf("stop waiting...\n");
    if(events_num<0||errno==EINTR){
        printf("poll exception!\n");
        throw std::exception();
    }

    for(int i=0;i<events_num;i++){
        pthread_mutex_lock(&hashmap_lock);
        std::shared_ptr<Channel> temp_channel=channels.find(events_set[i].data.fd)->second;
        pthread_mutex_unlock(&hashmap_lock);
        if(!temp_channel){
            printf("poll exception!\n");
            throw std::exception();
        }
        temp_channel->SetRevents(events_set[i].events);
        //printf("listendfd is %d,fd is %d,EPOLLIN is %d\n",temp_channel->GetListenfd(),temp_channel->Getfd(),temp_channel->GetRevents()&EPOLLIN);
        active_channels->push_back(temp_channel);
    }

    delete [] events_set;
}

void EpollPoller::AddChannel(std::shared_ptr<Channel> channel){
    int fd=channel->Getfd();
    pthread_mutex_lock(&hashmap_lock);
    channels.insert({fd,channel});
    pthread_mutex_unlock(&hashmap_lock);
    channel_num++;
    epoll_event e_event;
    e_event.data.fd=fd;
    e_event.events=channel->GetEvents();
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&e_event);
}

void EpollPoller::Update(int fd,int events){

    epoll_event e_event;
    e_event.data.fd=fd;
    e_event.events=events;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&e_event);
}

void EpollPoller::DelChannel(std::shared_ptr<Channel> channel){
    int fd=channel->Getfd();
    pthread_mutex_lock(&hashmap_lock);
    channels.erase(fd);
    pthread_mutex_unlock(&hashmap_lock);
    channel_num--;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
    close(fd);
}

std::shared_ptr<Channel> EpollPoller::FindChannel(int fd){
    
    pthread_mutex_lock(&hashmap_lock);
    std::unordered_map<int,std::shared_ptr<Channel>>::iterator it=channels.find(fd);
    std::shared_ptr<Channel> temp_channel;
    if(it==channels.end()){
        //重复删除
        printf("delete already!\n");
    }else{
        temp_channel=it->second;
    }
    pthread_mutex_unlock(&hashmap_lock);

    return temp_channel;
}
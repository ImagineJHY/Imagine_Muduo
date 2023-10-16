#include "Imagine_Muduo/Poller.h"

#include <memory>

#include "Imagine_Muduo/EventLoop.h"
#include "Imagine_Muduo/Channel.h"

namespace Imagine_Muduo
{

EpollPoller::EpollPoller(EventLoop *loop)
{
    loop_ = loop;
    channel_num_ = 0;
    epollfd_ = epoll_create(100);

    if (pthread_mutex_init(&hashmap_lock_, nullptr) != 0) {
        throw std::exception();
    }
}

void EpollPoller::poll(int timeoutMs, std::vector<std::shared_ptr<Channel>> *active_channels)
{
    epoll_event *events_set = new epoll_event[channel_num_];
    int events_num = epoll_wait(epollfd_, events_set, channel_num_, timeoutMs);
    // printf("stop waiting...\n");
    if (events_num < 0 || errno == EINTR) {
        LOG_INFO("poll exception!");
        throw std::exception();
    }

    for (int i = 0; i < events_num; i++) {
        pthread_mutex_lock(&hashmap_lock_);
        std::shared_ptr<Channel> temp_channel = channels_.find(events_set[i].data.fd)->second;
        pthread_mutex_unlock(&hashmap_lock_);
        if (!temp_channel) {
            LOG_INFO("poll exception!");
            throw std::exception();
        }
        temp_channel->SetRevents(events_set[i].events);
        // printf("listendfd is %d,fd is %d,EPOLLIN is %d\n",temp_channel->GetListenfd(),temp_channel->Getfd(),temp_channel->GetRevents()&EPOLLIN);
        active_channels->push_back(temp_channel);
    }

    delete[] events_set;
}

void EpollPoller::AddChannel(std::shared_ptr<Channel> channel)
{
    int fd = channel->Getfd();
    pthread_mutex_lock(&hashmap_lock_);
    channels_.insert({fd, channel});
    pthread_mutex_unlock(&hashmap_lock_);
    channel_num_++;
    epoll_event e_event;
    e_event.data.fd = fd;
    e_event.events = channel->GetEvents();
    epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &e_event);
}

void EpollPoller::Update(int fd, int events)
{
    epoll_event e_event;
    e_event.data.fd = fd;
    e_event.events = events;
    epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &e_event);
}

void EpollPoller::DelChannel(std::shared_ptr<Channel> channel)
{
    int fd = channel->Getfd();
    pthread_mutex_lock(&hashmap_lock_);
    channels_.erase(fd);
    pthread_mutex_unlock(&hashmap_lock_);
    channel_num_--;
    epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

std::shared_ptr<Channel> EpollPoller::FindChannel(int fd)
{
    pthread_mutex_lock(&hashmap_lock_);
    std::unordered_map<int, std::shared_ptr<Channel>>::iterator it = channels_.find(fd);
    std::shared_ptr<Channel> temp_channel;
    if (it == channels_.end()) {
        // 重复删除
        LOG_INFO("delete already!");
    } else {
        temp_channel = it->second;
    }
    pthread_mutex_unlock(&hashmap_lock_);

    return temp_channel;
}

} // namespace Imagine_Muduo
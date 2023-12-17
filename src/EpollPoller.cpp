#include "Imagine_Muduo/EpollPoller.h"

#include "Imagine_Muduo/log_macro.h"
#include "Imagine_Muduo/EventLoop.h"
#include "Imagine_Muduo/Channel.h"

namespace Imagine_Muduo
{

EpollPoller::EpollPoller(const EventLoop *loop) : Poller()
{
    loop_ = loop;
    channel_num_ = 0;
    epollfd_ = epoll_create(100);

    hashmap_lock_ = new pthread_mutex_t;
    if (pthread_mutex_init(hashmap_lock_, nullptr) != 0) {
        throw std::exception();
    }
}

Poller* EpollPoller::poll(int timeoutMs, std::vector<std::shared_ptr<Channel>>& active_channels)
{
    epoll_event *events_set = new epoll_event[channel_num_];
    int events_num = epoll_wait(epollfd_, events_set, channel_num_, timeoutMs);
    IMAGINE_MUDUO_LOG("stop waiting...");
    if (events_num < 0 || errno == EINTR) {
        IMAGINE_MUDUO_LOG("poll exception!");
        throw std::exception();
    }

    for (int i = 0; i < events_num; i++) {
        pthread_mutex_lock(hashmap_lock_);
        std::shared_ptr<Channel> temp_channel = channels_.find(events_set[i].data.fd)->second;
        pthread_mutex_unlock(hashmap_lock_);
        if (!temp_channel) {
            IMAGINE_MUDUO_LOG("poll exception!2");
            throw std::exception();
        }
        temp_channel->SetRevents(events_set[i].events);
        active_channels.push_back(temp_channel);
    }

    delete[] events_set;

    return this;
}

Poller* EpollPoller::AddChannel(const std::shared_ptr<Channel>& channel)
{
    int fd = channel->Getfd();
    pthread_mutex_lock(hashmap_lock_);
    channels_.insert(std::make_pair(fd, channel));
    pthread_mutex_unlock(hashmap_lock_);
    channel_num_++;
    epoll_event e_event;
    e_event.data.fd = fd;
    e_event.events = channel->GetEvents();
    epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &e_event);

    return this;
}

const Poller* EpollPoller::Update(int fd, int events) const
{
    epoll_event e_event;
    e_event.data.fd = fd;
    e_event.events = events;
    epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &e_event);

    return this;
}

Poller* EpollPoller::DelChannel(const std::shared_ptr<Channel>& channel)
{
    int fd = channel->Getfd();
    pthread_mutex_lock(hashmap_lock_);
    channels_.erase(fd);
    pthread_mutex_unlock(hashmap_lock_);
    channel_num_--;
    epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
    close(fd);

    return this;
}

std::shared_ptr<Channel> EpollPoller::FindChannel(int fd) const
{
    pthread_mutex_lock(hashmap_lock_);
    std::unordered_map<int, std::shared_ptr<Channel>>::const_iterator it = channels_.find(fd);
    std::shared_ptr<Channel> temp_channel;
    if (it == channels_.end()) {
        // 重复删除
        IMAGINE_MUDUO_LOG("delete already!");
    } else {
        temp_channel = it->second;
    }
    pthread_mutex_unlock(hashmap_lock_);

    return temp_channel;
}

} // namespace Imagine_Muduo
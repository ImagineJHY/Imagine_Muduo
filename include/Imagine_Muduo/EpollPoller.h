#ifndef IMAGINE_MUDUO_EPOLLPOLLER_H
#define IMAGINE_MUDUO_EPOLLPOLLER_H

#include "Poller.h"

#include <sys/epoll.h>
#include <unordered_map>
#include <errno.h>

namespace Imagine_Muduo
{

class EventLoop;

class EpollPoller : public Poller
{
 public:
    EpollPoller(const EventLoop *loop);

    Poller* poll(int timeoutMs, std::vector<std::shared_ptr<Channel>>& active_channels);

    Poller* AddChannel(const std::shared_ptr<Channel>& channel);

    Poller* DelChannel(const std::shared_ptr<Channel>& channel);

    const Poller* Update(int fd, int events) const;

    std::shared_ptr<Channel> FindChannel(int fd) const;

 private:
    int epollfd_;
    int channel_num_;
    pthread_mutex_t* hashmap_lock_;
    std::unordered_map<int, std::shared_ptr<Channel>> channels_;
    const EventLoop *loop_;
};

} // namespace Imagine_Muduo

#endif
#ifndef IMAGINE_MUDUO_POLLER_H
#define IMAGINE_MUDUO_POLLER_H

#include <sys/epoll.h>
#include <unordered_map>
#include <errno.h>
#include <vector>
#include <memory>

namespace Imagine_Muduo
{

class Channel;
class EventLoop;

class Poller
{

 public:
    virtual void poll(int timeoutMs, std::vector<std::shared_ptr<Channel>> *active_channels) = 0;
    virtual ~Poller(){};
    virtual void AddChannel(std::shared_ptr<Channel> channel) = 0;
    virtual void DelChannel(std::shared_ptr<Channel> channel) = 0;
    virtual void Update(int fd, int events) = 0;
    virtual std::shared_ptr<Channel> FindChannel(int fd) = 0;
};

class EpollPoller : public Poller
{
 public:
    EpollPoller(EventLoop *loop);

    void poll(int timeoutMs, std::vector<std::shared_ptr<Channel>> *active_channels);

    void AddChannel(std::shared_ptr<Channel> channel);

    void DelChannel(std::shared_ptr<Channel> channel);

    void Update(int fd, int events);

    std::shared_ptr<Channel> FindChannel(int fd);

 private:
    int epollfd_;
    int channel_num_;
    pthread_mutex_t hashmap_lock_;
    std::unordered_map<int, std::shared_ptr<Channel>> channels_;
    EventLoop *loop_;
};

} // namespace Imagine_Muduo

#endif
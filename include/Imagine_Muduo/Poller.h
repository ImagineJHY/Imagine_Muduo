#ifndef IMAGINE_MUDUO_POLLER_H
#define IMAGINE_MUDUO_POLLER_H

#include <vector>
#include <memory>

namespace Imagine_Muduo
{

class Channel;

class Poller
{
 public:
    Poller();
    
    virtual ~Poller();

    virtual Poller* poll(int timeoutMs, std::vector<std::shared_ptr<Channel>>& active_channels) = 0;

    virtual Poller* AddChannel(const std::shared_ptr<Channel>& channel) = 0;

    virtual Poller* DelChannel(const std::shared_ptr<Channel>& channel) = 0;

    virtual const Poller* Update(int fd, int events) const = 0;

    virtual std::shared_ptr<Channel> FindChannel(int fd) const = 0;
};

} // namespace Imagine_Muduo

#endif
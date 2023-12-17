#ifndef IMAGINE_MUDUO_CHANNEL_H
#define IMAGINE_MUDUO_CHANNEL_H

#include "common_typename.h"

#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <memory>

namespace Imagine_Muduo
{

class EventLoop;
class Buffer;

class Channel
{
 public:
   enum ChannelTyep
   {
      ListenChannel,
      EventChannel,
      TimerChannel
   };

 public:
    Channel();

    ~Channel();

    Channel* MakeSelf(std::shared_ptr<Channel> self);

    Channel* EnableRead();

    Channel* EnableWrite();

    Channel* DisableRead();

    Channel* DisableWrite();

    Channel* SetRevents(int revents);

    Channel* SetEvents(int events);

    int GetRevents() const;

    int GetEvents() const;

    std::string GetPeerIp() const;

    std::string GetPeerPort() const;

    Channel* ParsePeerAddr();

    Channel* Setfd(int fd);

    int Getfd() const;

    Channel* SetLoop(EventLoop *loop);

    EventLoop *GetLoop() const;

    Channel* SetListenfd(int fd);

    static void SetNonBlocking(int fd);

    static std::shared_ptr<Channel> Create(EventLoop *loop, int value, ChannelTyep type = EventChannel);

    void Close();

    void HandleEvent();

    void DefaultEventHandler();

    void DefaultTimerfdReadEventHandler();

    Channel* SetReadHandler(EventHandler read_handler);

    Channel* SetWriteHandler(EventHandler write_handler);

    Channel* SetEventHandler(EventHandler handler);

 private:
    void Init();

    void Update() const;

 private:
    int fd_;
    int listen_fd_;
    int events_;
    int revents_;
    EventLoop *loop_;
    std::shared_ptr<Channel> self_;

    std::string peer_ip_;
    std::string peer_port_;

    EventHandler handler_;
    EventHandler read_handler_;
    EventHandler write_handler_;
};

} // namespace Imagine_Muduo

#endif
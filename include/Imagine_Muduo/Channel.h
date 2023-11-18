#ifndef IMAGINE_MUDUO_CHANNEL_H
#define IMAGINE_MUDUO_CHANNEL_H

#include "Imagine_Muduo/EventLoop.h"
#include "Imagine_Muduo/Buffer.h"

#include <string>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <algorithm>

namespace Imagine_Muduo
{

class EventLoop;
class Buffer;

class Channel
{
 public:
   enum ChannelTyep{
      ListenChannel,
      EventChannel,
      TimerChannel
   };

 public:
    // Channel();

    ~Channel();

    void MakeSelf(std::shared_ptr<Channel> self);

    void SetReadCallback(ChannelCallback callback);

    void SetWriteCallback(ChannelCallback callback);

    void SetCloseCallback(ChannelCallback callback);

    void SetErrorCallback(ChannelCallback callback);

    void SetCommunicateCallback(EventCommunicateCallback callback);

    void EnableRead();

    void EnableWrite();

    void DisableRead();

    void DisableWrite();

    int SetRevents(int revents);

    int SetEvents(int events);

    int GetRevents();

    int GetEvents();

    std::string GetPeerIp() const;

    std::string GetPeerPort() const;

    // void HandleEvent();

    bool Process();

    void ParsePeerAddr();

    void ProcessIovec(struct iovec *io_block);

    void Setfd(int fd);

    int Getfd();

    void SetAddr(struct sockaddr_in &addr);

    struct sockaddr_in GetAddr();

    void SetLoop(EventLoop *loop);

    EventLoop *GetLoop();

    void SetListenfd(int fd)
    {
        listen_fd_ = fd;
    }

    static void SetNonBlocking(int fd);

    static std::shared_ptr<Channel> Create(EventLoop *loop, int value, ChannelTyep type = EventChannel);

    static void Destroy(std::shared_ptr<Channel> channel);

    bool Send(struct iovec *send_data, int len);

    void Close();

    void HandleEvent();

    void DefaultEventHandler();

    void DefaultListenfdReadEventHandler();

    void DefaultEventfdReadEventHandler();

    void DefaultTimerfdReadEventHandler();

    void DefaultEventfdWriteEventHandler();

    void SetReadHandler(EventHandler read_handler);

    void SetWriteHandler(EventHandler write_handler);

    bool SetEventHandler(EventHandler handler);

    bool SetReadEventHandler(EventHandler read_handler);

    bool SetWriteEventHandler(EventHandler write_handler);

 private:
    void Update();

 private:
    int fd_;
    int listen_fd_;
    int events_;
    int revents_;
    bool alive_ = false;
    bool clear_readbuf_;
    int read_or_write_;
    struct sockaddr_in client_addr_;
    EventLoop *loop_;
    std::shared_ptr<Channel> self_;

    std::string peer_ip_;
    std::string peer_port_;

    ChannelCallback read_callback_ = nullptr;
    ChannelCallback write_callback_ = nullptr;
    ChannelCallback close_callback_ = nullptr;
    ChannelCallback error_callback_ = nullptr;

    EventCommunicateCallback communicate_callback_ = nullptr;

    EventHandler handler_ = nullptr;
    EventHandler read_handler_ = nullptr;
    EventHandler write_handler_ = nullptr;

    bool write_flag_ = false;          // 标识写是否完成
    bool write_callback_flag_ = false; // 标识是否执行过write_callback
};

} // namespace Imagine_Muduo

#endif
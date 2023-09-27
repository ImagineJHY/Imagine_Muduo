#ifndef IMAGINE_MUDUO_CHANNEL_H
#define IMAGINE_MUDUO_CHANNEL_H

#include <string>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <algorithm>

#include "EventLoop.h"
#include "Buffer.h"

namespace Imagine_Muduo
{

class EventLoop;
class Buffer;

class Channel
{

    // public:
    //     using EventHandler=std::function<void()>;
    //     using EventCallback=std::function<struct iovec*(const struct iovec*)>;
    //     using EventCommunicateCallback=std::function<bool(const char*,int)>;//粘包判断函数

 public:
    // Channel();

    ~Channel();

    void MakeSelf(std::shared_ptr<Channel> self);

    void SetReadCallback(EventCallback callback);

    void SetWriteCallback(EventCallback callback);

    void SetCloseCallback(EventCallback callback);

    void SetErrorCallback(EventCallback callback);

    void SetCommunicateCallback(EventCommunicateCallback callback);

    void EnableRead();

    void EnableWrite();

    void DisableRead();

    void DisableWrite();

    int SetRevents(int revents);

    int SetEvents(int events);

    int GetRevents();

    int GetEvents();

    // void HandleEvent();

    bool Process();

    void InitIovec(struct iovec *read_str, struct sockaddr_in *addr, bool get_read_buf = true);

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

    static std::shared_ptr<Channel> Create(EventLoop *loop, int value, int type = 1);

    static void Destroy(std::shared_ptr<Channel> channel);

    bool Send(struct iovec *send_data, int len);

    void Close();

    void HandleEvent();

    void DefaultEventHandler();

    void DefaultListenfdReadEventHandler();

    void DefaultEventfdReadEventHandler();

    void DefaultTimerfdReadEventHandler();

    void DefaultEventfdWriteEventHandler();

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

    EventCallback read_callback_ = nullptr;
    EventCallback write_callback_ = nullptr;
    EventCallback close_callback_ = nullptr;
    EventCallback error_callback_ = nullptr;

    EventCommunicateCallback communicate_callback_ = nullptr;

    EventHandler handler_ = nullptr;
    EventHandler read_handler_ = nullptr;
    EventHandler write_handler_ = nullptr;

    Buffer read_buffer_;
    Buffer write_buffer_;

    struct iovec *read_iovec_;
    struct iovec *write_iovec_ = nullptr;
    bool write_flag_ = false;          // 标识写是否完成
    bool write_callback_flag_ = false; // 标识是否执行过write_callback
};

} // namespace Imagine_Muduo

#endif
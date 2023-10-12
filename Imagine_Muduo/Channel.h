#ifndef IMAGINE_MUDUO_CHANNEL_H
#define IMAGINE_MUDUO_CHANNEL_H

#include<string>
#include<functional>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<fcntl.h>
#include<algorithm>

#include"EventLoop.h"
#include"Buffer.h"

namespace Imagine_Muduo{


class EventLoop;
class Buffer;

class Channel{

// public:
//     using EventHandler=std::function<void()>;
//     using EventCallback=std::function<struct iovec*(const struct iovec*)>;
//     using EventCommunicateCallback=std::function<bool(const char*,int)>;//粘包判断函数

public:
    // Channel();

    ~Channel();
    
    void MakeSelf(std::shared_ptr<Channel> self_);

    void SetReadCallback(EventCallback callback);

    void SetWriteCallback(EventCallback callback);

    void SetCloseCallback(EventCallback callback);

    void SetErrorCallback(EventCallback callback);

    void SetCommunicateCallback(EventCommunicateCallback callback);

    void EnableRead();

    void EnableWrite();

    void DisableRead();

    void DisableWrite();

    int SetRevents(int revents_);

    int SetEvents(int events_);

    int GetRevents();

    int GetEvents();

    //void HandleEvent();

    bool Process();

    void InitIovec(struct iovec* read_str,struct sockaddr_in* addr, bool get_read_buf_=true);

    void ProcessIovec(struct iovec* io_block);

    void Setfd(int fd_);

    int Getfd();

    void SetAddr(struct sockaddr_in& addr);

    struct sockaddr_in GetAddr();

    void SetLoop(EventLoop* loop_);

    EventLoop* GetLoop();

    void SetListenfd(int fd_){
        listen_fd=fd_;
    }

    static void SetNonBlocking(int fd);

    static std::shared_ptr<Channel> Create(EventLoop* loop_,int value,int type=1);

    static void Destroy(std::shared_ptr<Channel> channel);

    bool Send(struct iovec* send_data, int len);

    void Close();

    void HandleEvent();

    void DefaultEventHandler();

    void DefaultListenfdReadEventHandler();

    void DefaultEventfdReadEventHandler();

    void DefaultTimerfdReadEventHandler();

    void DefaultEventfdWriteEventHandler();

    bool SetEventHandler(EventHandler handler_);

    bool SetReadEventHandler(EventHandler read_handler_);

    bool SetWriteEventHandler(EventHandler write_handler_);

private:
    void Update();

private:
    int fd;
    int listen_fd;
    int events;
    int revents;
    bool alive=false;
    bool clear_readbuf;
    int read_or_write;
    struct sockaddr_in client_addr;
    EventLoop* loop;
    std::shared_ptr<Channel> self;

    EventCallback read_callback=nullptr;
    EventCallback write_callback=nullptr;
    EventCallback close_callback=nullptr;
    EventCallback error_callback=nullptr;

    EventCommunicateCallback communicate_callback=nullptr;

    EventHandler handler=nullptr;
    EventHandler read_handler=nullptr;
    EventHandler write_handler=nullptr;

    Buffer read_buffer;
    Buffer write_buffer;

    struct iovec* read_iovec;
    struct iovec* write_iovec=nullptr;
    bool write_flag=false;//标识写是否完成
    bool write_callback_flag=false;//标识是否执行过write_callback
};



}



#endif
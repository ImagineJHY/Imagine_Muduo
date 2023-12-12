#include "Imagine_Muduo/Channel.h"

#include "Imagine_Muduo/EventLoop.h"
#include "Imagine_Muduo/Buffer.h"

namespace Imagine_Muduo
{

Channel::Channel()
{
    Init();
}

Channel::~Channel()
{
}

void Channel::Init()
{
    handler_ = nullptr;
    read_handler_ = nullptr;
    write_handler_ = nullptr;
}

Channel* Channel::MakeSelf(std::shared_ptr<Channel> self)
{
    self_ = self;

    return this;
}

Channel* Channel::EnableRead()
{
    events_ |= EPOLLIN;
    Update();

    return this;
}

Channel* Channel::EnableWrite()
{
    events_ |= EPOLLOUT;
    Update();

    return this;
}

Channel* Channel::DisableRead()
{
    events_ &= ~EPOLLIN;
    Update();

    return this;
}

Channel* Channel::DisableWrite()
{
    events_ &= ~EPOLLOUT;
    Update();

    return this;
}

Channel* Channel::SetRevents(int revents)
{
    revents_ = revents;

    return this;
}

Channel* Channel::SetEvents(int events)
{
    events_ = events;
    Update();

    return this;
}

int Channel::GetRevents() const
{
    return revents_;
}

int Channel::GetEvents() const
{
    return events_;
}

std::string Channel::GetPeerIp() const
{
    return peer_ip_;
}

std::string Channel::GetPeerPort() const
{
    return peer_port_;
}

Channel* Channel::ParsePeerAddr()
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(&addr);
    getpeername(fd_, (struct sockaddr *)&addr, &addr_size);
    char* ptr = inet_ntoa(addr.sin_addr);
    while (*ptr != '\0') {
        peer_ip_.push_back(*ptr);
        ptr++;
    }
    peer_port_ = std::to_string(ntohs(addr.sin_port));

    return this;
}

Channel* Channel::Setfd(int fd)
{
    fd_ = fd;

    return this;
}

int Channel::Getfd() const
{
    return fd_;
}

Channel* Channel::SetLoop(EventLoop *loop)
{
    loop_ = loop;

    return this;
}

EventLoop *Channel::GetLoop() const
{
    return loop_;
}

Channel* Channel::SetListenfd(int fd)
{
    listen_fd_ = fd;

    return this;
}

void Channel::SetNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

std::shared_ptr<Channel> Channel::Create(EventLoop *loop, int value, ChannelTyep type)
{

    if (value < 0) {
        throw std::exception();
    }

    int reuse = 1;
    int sockfd;
    struct sockaddr_in saddr;
    int events;
    int listenfd;
    std::shared_ptr<Channel> new_channel = std::make_shared<Channel>();
    new_channel->SetEventHandler(std::bind(&Channel::DefaultEventHandler, new_channel.get()));

    if (type == EventChannel) { // 创建通信Channel
        listenfd = value;
        socklen_t saddr_len = sizeof(saddr);
        sockfd = accept(listenfd, (struct sockaddr *)&saddr, &saddr_len);
        if (sockfd < 0) {
            if (errno == EAGAIN) {
                return nullptr;
            }
            LOG_INFO("create channel exception");
            throw std::exception();
        }
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)); // 设置端口复用
        events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
    } else if (type == TimerChannel) { // 创建timerChannel

        new_channel->SetReadHandler(std::bind(&Channel::DefaultTimerfdReadEventHandler, new_channel.get()));
        sockfd = TimeUtil::CreateTimer();
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)); // 设置端口复用
        events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET;
        listenfd = 0;
    } else { // 创建监听Channel
        listenfd = sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            LOG_INFO("create channel exception2");
            throw std::exception();
        }

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)); // 设置端口复用

        saddr.sin_port = htons(value);
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = INADDR_ANY;
        bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)); // 绑定端口

        if (listen(sockfd, 128) == -1) {
            LOG_INFO("Create listen exception!");
            throw std::exception();
        }
        events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
    }

    SetNonBlocking(sockfd);
    new_channel->MakeSelf(new_channel);
    new_channel->SetLoop(loop);
    new_channel->Setfd(sockfd);
    new_channel->SetEvents(events);
    new_channel->SetListenfd(listenfd);

    return new_channel;
}

void Channel::Update() const
{
    loop_->UpdateChannel(self_);
}

void Channel::Close()
{
    loop_->CloseChannel(self_);
    self_.reset();
}

void Channel::HandleEvent()
{
    this->handler_();
}

void Channel::DefaultEventHandler()
{
    if ((revents_ & EPOLLIN) && read_handler_) {
        read_handler_();
    } else if ((revents_ & EPOLLOUT) && write_handler_) {
        write_handler_();
    }
}

void Channel::DefaultTimerfdReadEventHandler()
{
    TimeStamp now(NOW_MS);
    TimeUtil::ReadTimerfd(this->Getfd());
    std::vector<Timer *> expired_timers = GetLoop()->GetExpiredTimers(now);
    SetEvents(EPOLLIN | EPOLLONESHOT | EPOLLRDHUP | EPOLLET);
    for (size_t i = 0; i < expired_timers.size(); i++) {
        if (!expired_timers[i]->IsAlive()) {
            delete expired_timers[i];
        } else {
            expired_timers[i]->RunCallback();
            if (expired_timers[i]->ResetCallTime()) {
                GetLoop()->InsertTimer(expired_timers[i]);
            } else {
                GetLoop()->CloseTimer(expired_timers[i]->GetTimerId());
                delete expired_timers[i];
            }
        }
    }
}

Channel* Channel::SetReadHandler(EventHandler read_handler)
{
    read_handler_ = read_handler;

    return this;
}

Channel* Channel::SetWriteHandler(EventHandler write_handler)
{
    write_handler_ = write_handler;

    return this;
}

Channel* Channel::SetEventHandler(EventHandler handler)
{
    handler_ = handler;

    return this;
}

} // namespace Imagine_Muduo